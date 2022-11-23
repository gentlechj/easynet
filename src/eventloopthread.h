#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "eventloop.h"
#include "util.h"

namespace easynet {

class EventLoopThread : private noncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();
  EventLoop* m_loop;
  bool m_exiting;

  std::thread m_thread;
  std::mutex m_mutex;
  std::condition_variable m_cv;
};
}  // namespace easynet