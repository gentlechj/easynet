#include "eventloopthread.h"

namespace easynet {

EventLoopThread::EventLoopThread() : m_loop(nullptr), m_exiting(false) {}

EventLoopThread::~EventLoopThread() {
  m_exiting = true;
  m_loop->quit();
  if (m_thread.joinable()) m_thread.join();
}

void EventLoopThread::threadFunc() {
  EventLoop loop;

  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_loop = &loop;
    m_cv.notify_one();
  }

  loop.loop();
}

EventLoop* EventLoopThread::startLoop() {
  assert(!m_thread.joinable());
  m_thread = std::thread(std::bind(&EventLoopThread::threadFunc, this));

  {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_loop == nullptr) {
      m_cv.wait(lock);
    }
  }

  return m_loop;
}
}  // namespace easynet