#include "eventloop.h"

#include <stdio.h>

#include <chrono>
#include <thread>

#include "channel.h"
#include "util.h"

#ifdef OS_MACOSX
#include "./timefd.h"
#elif defined(OS_LINUX)
#include <sys/timerfd.h>
#endif

using namespace easynet;
using namespace std;

EventLoop* g_loop;

void timeout() {
  printf("Timeout!\n");
  g_loop->quit();
}
int main() {
  printf("main(): pid = %d, tid = %d\n", getpid(), gettid());
  EventLoop loop;
  g_loop = &loop;

#ifdef OS_MACOSX
  TimerFd ctimerfd;
  int timerfd = ctimerfd.getFd();
#elif defined(OS_LINUX)
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
#endif

  Channel channel(&loop, timerfd);
  channel.setReadCallback(timeout);
  channel.enableRead();

#ifdef OS_MACOSX
  ctimerfd.setTime(std::chrono::milliseconds(5000));
#elif defined(OS_LINUX)
  struct itimerspec itimerspec;
  bzero(&itimerspec, sizeof(itimerspec));
  itimerspec.it_value.tv_sec = 5;
  ::timerfd_settime(timerfd, 0, &itimerspec, NULL);
#endif
  loop.loop();

#ifdef OS_LINUX
  ::close(timerfd);
#endif
  return 0;
}