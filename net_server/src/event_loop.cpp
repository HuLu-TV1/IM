#include "event_loop.h"
#include "channel.h"
#include "epoll.h"
#include "log.h"
#include <algorithm>

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include "time_wheel.h"
using namespace net;
const int thread_num = 8;

int CreateEvenFd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    LOG_DEBUG("Failed in eventfd");
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop(std::string name) : name_(name),
                                         quit_(false),
                                         threadId_(std::this_thread::get_id()),
                                         callingPendingFunctors_(false)
                                         
{
  epoll_ = std::make_unique<Epoll>();
  //timer_ = std::make_unique<TimerLoop>(this);
  timer_ = new TimerLoop(this);
  wake_fd_ = CreateEvenFd();
  wake_channel_ = std::make_unique<Channel>(this, wake_fd_);
  std::function<void()> cb = std::bind(&EventLoop::HandleRead, this);
  wake_channel_->set_handle_read_event(cb);
  wake_channel_->EnableRead();
}

void EventLoop::HandleRead()
{
  uint64_t one = 1;
  ssize_t n = ::read(wake_fd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_DEBUG("EventLoop::handleRead() reads %d bytes instead of 8", n);
  }
}

void EventLoop::WakeUp()
{
  uint64_t one = 1;
  ssize_t n = ::write(wake_fd_, &one, sizeof one);
  LOG_DEBUG("WakeUp");
  if (n != sizeof one)
  {
    LOG_DEBUG("EventLoop::handleRead() reads %d bytes instead of 8", n);
  }
}


EventLoop::~EventLoop()
{
  wake_channel_->DisableAll();
  ::close(wake_fd_);
}
void EventLoop::Loop()
{
  while (!quit_)
  {
    std::vector<Channel *> chs;
    chs = epoll_->CreateWait();
    for (auto it : chs)
    {
      LOG_DEBUG("channel fd = %d",it->get_fd());
      it->HandleEvent();
    }
    doPendingFunctors();
  }
}

void EventLoop::UpdateChannel(Channel *channel) { epoll_->Update(channel); }
void EventLoop::abortNotInLoopThread()
{

  LOG_DEBUG("EventLoop::abortNotInLoopThread - EventLoop  = %p  was created in threadId_ = %d , current thread id = %d", this, threadId_, std::this_thread::get_id());
}

void EventLoop::RunInLoop(Functor cb)
{
  if (isInLoopThread())
  {
    cb();
  }
  else
  {
    QueueInLoop(std::move(cb));
  }
}
void EventLoop::QueueInLoop(Functor cb)
{
  {
    std::lock_guard<std::mutex> lk(mut);
    pendingFunctors_.push_back(cb);
    if (!isInLoopThread() ||callingPendingFunctors_)
    {
      WakeUp();
    }
  }
}
size_t EventLoop::QueueSize() const
{
  std::lock_guard<std::mutex> lk(mut);
  return pendingFunctors_.size();
}

void EventLoop::doPendingFunctors()
{
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;
  {
    std::lock_guard<std::mutex> lk(mut);
    functors.swap(pendingFunctors_);
  }
  LOG_DEBUG("functors size = %d", functors.size());
  for (const Functor &functor : functors)
  {
    functor();
  }
  callingPendingFunctors_ = false;
}


void EventLoop:: Timer_Once(double time,TimerCallback cb) {
  itimerspec when ;
  when.it_value.tv_sec = time/1000;
  when.it_value.tv_nsec =time;
  timer_->addTimer(std::move(cb),when);

}
void EventLoop:: Timer_Every(double time,TimerCallback cb) {
  itimerspec when ;
  when.it_value.tv_sec = time/1000;
  when.it_value.tv_nsec =time;
  when.it_interval.tv_sec =  time/1000;
   when.it_interval.tv_nsec =time;
  timer_->addTimer(std::move(cb),when);
}
