#ifndef __EVENTLOOP_
#define __EVENTLOOP_
#include "noncopyable.h"
#include "thread_pool.h"
#include <functional>
#include <memory>
#include <mutex>
#include <string.h>
#include "callback.h"
namespace net
{
  class Epoll;
  class Channel;
class TimerLoop;
  class EventLoop : noncopyable
  {
  public:
    typedef std::function<void()> Functor;
    EventLoop(std::string name);
    void Loop();
    ~EventLoop();
    void UpdateChannel(Channel *channel);
    void AddThreadPool(Task task);
    void assertInLoopThread()
    {
      if (!isInLoopThread())
      {
        abortNotInLoopThread();
      }
    }

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }
    void RunInLoop(Functor cb);
    void QueueInLoop(Functor cb);
    size_t QueueSize() const;
    void doPendingFunctors();
    void Timer_Once(double time,TimerCallback cb);
    void Timer_Every(double time,TimerCallback cb);
    std::string name_;

  private:
    void abortNotInLoopThread();
    std::unique_ptr<Epoll> epoll_;
    bool quit_;
     std::thread::id threadId_;
    std::vector<Functor> pendingFunctors_;
    mutable std::mutex mut;
    int wake_fd_;
    std::unique_ptr<Channel>wake_channel_;
     void HandleRead();  // waked up
     void WakeUp();
    std::atomic<bool> callingPendingFunctors_;
   // std::unique_ptr<TimerLoop> timer_;
   TimerLoop *timer_;
  };
} // namespace net
#endif