#include <stdio.h>

#include <chrono>
#include <thread>

#include "channel.h"
#include "eventloop.h"
#include "timerfd.h"
#include "util.h"
#include "logging.h"

using namespace easynet;
using namespace std;

EventLoop* g_loop;

void timeout(TimeStamp receiveTime) {
  printf("%s Timeout!\n", readableTime(receiveTime).c_str());
  g_loop->quit();
}

int main() {
//   setloglevel("trace");
  printf("main(): pid = %d, tid = %d\n", getpid(), gettid());
  printf("%s started!\n", readableTime(now()).c_str());
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