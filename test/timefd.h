#include <signal.h>
#include <sys/pipe.h>
#include <sys/poll.h>
#include <sys/time.h>

#include <atomic>
#include <thread>

#include "logging.h"
#include "util.h"
namespace easynet {

class TimerFd : private noncopyable {
 public:
  TimerFd();
  ~TimerFd();
  int getFd() const;
  void setTime(const std::chrono::milliseconds& timeout);
  void alarm() {
    ::write(m_pipeFd[1], " ", 1);
    m_isTiming = false;
  }

 private:
  int m_pipeFd[2];
  std::atomic<bool> m_isTiming;
};

TimerFd::TimerFd() : m_isTiming(false) {
  if (::pipe(m_pipeFd) == -1) {
    fatal("TimeFd not available");
  }
}

TimerFd::~TimerFd() {
  ::close(m_pipeFd[1]);
  ::close(m_pipeFd[0]);
}

int TimerFd::getFd() const { return m_pipeFd[0]; }

void TimerFd::setTime(const std::chrono::milliseconds& timeout) {
  if (m_isTiming) {
    error("Timer is running.");
    return;
  }

  auto timeoutMs = static_cast<int>(timeout.count());
  m_isTiming = true;

  std::thread timer([this, timeoutMs]() {
    // setitemer
    // struct itimerval myit = {{0, 0}, {0, 0}};
    // myit.it_value.tv_sec = timeoutMs / 1000;
    // setitimer(ITIMER_REAL, &myit, NULL);
    ::poll(nullptr, 0, timeoutMs);
    ::write(m_pipeFd[1], " ", 1);
    m_isTiming = false;
  });
  timer.detach();
}
}  // namespace easynet
