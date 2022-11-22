#include <stdio.h>

#include <chrono>
#include <thread>

#include "channel.h"
#include "eventloop.h"
#include "timerfd.h"
#include "util.h"

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

  TimerFd ctimerfd;
  int timerfd = ctimerfd.getFd();

  Channel channel(&loop, timerfd);
  channel.setReadCallback(timeout);
  channel.enableRead();

  ctimerfd.setTime(now() + 2500);
  loop.loop();

  return 0;
}