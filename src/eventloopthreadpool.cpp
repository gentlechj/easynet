#include "eventloopthreadpool.h"

#include "eventloop.h"
#include "eventloopthread.h"

namespace easynet {
EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
    : m_baseLoop(baseLoop), m_started(false), m_numThreads(0), m_next(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
  // 不需要删除loop
}

void EventLoopThreadPool::start() {
  assert(!m_started);
  m_baseLoop->assertInLoopThread();

  m_started = true;

  for (int i = 0; i < m_numThreads; ++i) {
    std::unique_ptr<EventLoopThread> t(new EventLoopThread);
    m_loops.push_back(t->startLoop());
    m_threads.push_back(std::move(t));
  }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
  m_baseLoop->assertInLoopThread();
  // 默认为主线程和IO线程为同一个线程
  EventLoop* loop = m_baseLoop;

  if (!m_loops.empty()) {
    // round-robin
    loop = m_loops[m_next];
    ++m_next;
    if (static_cast<size_t>(m_next) >= m_loops.size()) {
      m_next = 0;
    }
  }
  return loop;
}
}  // namespace easynet