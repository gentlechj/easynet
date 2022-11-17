#include "eventloop.h"
#include "logging.h"
#include <cassert>
#include <vector>

#ifdef OS_LINUX
#include <sys/epoll.h>
#elif defined(OS_MACOSX)
#include <sys/event.h>
#include <sys/poll.h>
#else
#error "platform unsupported"
#endif

namespace easynet {

static thread_local EventLoop *t_loopInThisThread = nullptr;

EventLoop::EventLoop() : m_looping(false), m_threadId(gettid()) {
  info("EventLoop created %p in thread %d", this, m_threadId);
  fatalif(t_loopInThisThread, "Another EventLoop %p exists in this thread %d",
          t_loopInThisThread, m_threadId);
  t_loopInThisThread = this;
}

EventLoop::~EventLoop() {
  assert(!m_looping);
  t_loopInThisThread = nullptr;
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

void EventLoop::abortNotInLoopThread() {
  fatal("EventLoop::abortNotInLoopThread - EventLoop %p was created in "
        "threadId = %d, current thread id = %d",
        this, m_threadId, gettid());
}

void EventLoop::loop() {
  assert(!m_looping);
  assertInLoopThread();
  m_looping = true;
  ::poll(nullptr, 0, 5 * 1000);
  info("EventLoop %p stop looping", this);
  m_looping = false;
}
} // namespace easynet