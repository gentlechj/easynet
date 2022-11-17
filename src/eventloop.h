#pragma once
#include "util.h"

namespace easynet {

class EventLoop : private noncopyable {
public:
  EventLoop();
  ~EventLoop();

  //进入事件处理循环
  void loop();

  bool isInLoopThread() const { return m_threadId == gettid(); }

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  static EventLoop *getEventLoopOfCurrentThread();

private:
  void abortNotInLoopThread();
  bool m_looping;
  const pid_t m_threadId;
};
} // namespace easynet