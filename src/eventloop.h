#pragma once
#include <cassert>
#include <memory>
#include <vector>

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
  void updateChannel(Channel* channel);
  static EventLoop *getEventLoopOfCurrentThread();

 private:
  void abortNotInLoopThread();
  typedef std::vector<Channel *> ChannelList;

  bool m_looping;
  bool m_quit;
  const pid_t m_threadId;
  std::unique_ptr<Poller> m_poller;
  ChannelList m_activeChannels;
};
}  // namespace easynet