
#include "timerfd.h"

#include <sys/pipe.h>
#include <sys/poll.h>
#ifdef OS_LINUX
#include <sys/timerfd.h
#endif

#include <thread>

#include "logging.h"
namespace easynet {

TimerFd::TimerFd() {
#ifdef OS_MACOSX
  if (::pipe(m_pipeFd) == -1) {
    fatal("TimeFd not available");
  }
  m_timerFd = m_pipeFd[0];
#elif defined(OS_LINUX)
  m_timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (m_timerFd < 0) {
    fatal("TimeFd not available");
  }
#endif
}

TimerFd::~TimerFd() {
#ifdef OS_MACOSX
  ::close(m_pipeFd[1]);
  ::close(m_pipeFd[0]);
#elif defined(OS_LINUX)
  ::close(m_timerFd);
#endif
}

int TimerFd::getFd() const { return m_timerFd; }

void TimerFd::alarm() {
  uint64_t one = 1;
  ssize_t n = ::write(m_pipeFd[1], &one, sizeof(one));
  if (n != sizeof(one)) {
    error("TimerFd::alarm() writes %ld bytes instead of 8", n);
  }
}
void TimerFd::read() {
  uint64_t one = 1;
  ssize_t n = ::read(m_timerFd, &one, sizeof(one));
  if (n != sizeof(one)) {
    error("TimerFd::read() reads %d bytes instead of 8", static_cast<int>(n));
  }
}

void TimerFd::setTime(int64_t timeStamp) {
  auto timeout = std::chrono::duration<int>(timeStamp - now());
  auto timeoutMs = static_cast<int>(timeout.count());

#ifdef OS_MACOSX
  std::thread timer([this, timeoutMs]() {
    // setitemer
    // struct itimerval myit = {{0, 0}, {0, 0}};
    // myit.it_value.tv_sec = timeoutMs / 1000;
    // setitimer(ITIMER_REAL, &myit, NULL);
    ::poll(nullptr, 0, timeoutMs);
    this->alarm();
  });
  timer.detach();
#elif defined(OS_LINUX)
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = timeoutMs;
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret) {
    error("timerfd_settime()");
  }
#endif
}
}  // namespace easynet