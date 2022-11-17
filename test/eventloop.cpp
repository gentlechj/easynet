#include "eventloop.h"
#include "util.h"
#include <stdio.h>
#include <thread>

using namespace easynet;
using namespace std;

void threadFunc() {
  printf("threadFunc(): pid = %d, tid = %d\n", getpid(), gettid());

  EventLoop loop;
  loop.loop();
}

int main() {
  printf("main(): pid = %d, tid = %d\n", getpid(), gettid());

  EventLoop loop;

  thread t(threadFunc);

  loop.loop();

  if (t.joinable())
    t.join();
  return 0;
}