#include <vector>

#include "util.h"
#include <memory>

namespace easynet {
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : private noncopyable {
 public:
  EventLoopThreadPool(EventLoop* baseLoop);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { m_numThreads = numThreads; }
  void start();
  EventLoop* getNextLoop();

 private:
  EventLoop* m_baseLoop;
  bool m_started;
  int m_numThreads;
  int m_next;  // number of next thread
  std::vector<EventLoop*> m_loops;
  std::vector<std::unique_ptr<EventLoopThread>> m_threads;
};
}  // namespace easynet