#pragma once
#include <atomic>
#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "timerManager.h"
#include "util.h"

#ifdef OS_LINUX
#include <sys/epoll.h>
#elif defined(OS_MACOSX)
#include <sys/event.h>
#include <sys/poll.h>
#else
#error "platform unsupported"
#endif
namespace easynet {
class Poller;
class Channel;
class EventLoop : private noncopyable {
 public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();

  // 进入事件处理循环
  void loop();

  bool isInLoopThread() const { return m_threadId == gettid(); }

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  void quit();
  void updateChannel(Channel *channel);
  static EventLoop *getEventLoopOfCurrentThread();

  TimerId runAt(const TimeStamp time, const TimerCallback &callback);
  TimerId runAfter(int64_t delay, const TimerCallback &callback);
  TimerId runEvery(const TimerInterval interval, const TimerCallback &callback);

  void runInloop(const Functor &callback);
  void queueInloop(const Functor &callback);
  void wakeup();

 private:
  typedef std::vector<Channel *> ChannelList;

  void abortNotInLoopThread();

  void handleRead();  // wakeup
  int createWakeupFd();

  void doPendingFunctors();

  bool m_looping;
  bool m_quit;
  const pid_t m_threadId;
  std::unique_ptr<Poller> m_poller;
  std::unique_ptr<Channel> m_wakeupChannel;
  std::unique_ptr<TimerManager> m_timerManager;

  ChannelList m_activeChannels;

  std::mutex m_mutex;
  std::atomic<bool> m_callingPendingFunctors;
  std::vector<Functor> m_pendingFunctors;

  int m_pipeFd[2];
  int m_wakeupFd;
};
}  // namespace easynet