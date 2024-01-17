#include "time_wheel.h"
#include <sys/timerfd.h>
#include "log.h"
#include "event_loop.h"
using namespace net;
int createTimerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    LOG_DEBUG("Failed in timerfd_create");
  }
  return timerfd;
}

void resetTimerfd(int timerfd,itimerspec when)
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;

 memcpy(&newValue,&when,sizeof(itimerspec));

  int ret = ::timerfd_settime(timerfd, 0, &newValue, nullptr);
  if (ret)
  {
     LOG_DEBUG("timerfd_settime()");
  }
}

TimerLoop ::TimerLoop(EventLoop *loop) 
: loop_(loop), timerfd_(createTimerfd()), timer_fd_channel_(loop, timerfd_)
{
  timer_fd_channel_.set_handle_read_event(std::bind(&TimerLoop::HandleRead,this));
  timer_fd_channel_.EnableRead();
}

 TimerLoop::~TimerLoop() {
  timer_fd_channel_.DisableAll();
  ::close(timerfd_);
   //delete all time
   //delete
 }


void  TimerLoop ::addTimer(TimerCallback cb,itimerspec  when) {
  cb_ = std::move(cb);
    loop_->RunInLoop(
      std::bind(&TimerLoop::addTimerInLoop, this,when ));
}

void TimerLoop ::addTimerInLoop(itimerspec when) {
  loop_->assertInLoopThread();
  resetTimerfd(timerfd_, when);
}

void TimerLoop ::HandleRead() {

  loop_->assertInLoopThread();
      uint64_t howmany;
  ssize_t n = ::read(timerfd_, &howmany, sizeof howmany);
    if (n != sizeof howmany)
  {
   LOG_DEBUG("EventLoop::handleRead() reads %d bytes instead of 8", n);
  }
  cb_();
}
