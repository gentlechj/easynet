#include <stdio.h>

#include "eventloop.h"
#include "eventloopthread.h"
#include "logging.h"
#include "util.h"

using namespace easynet;

void runInThread() {
  printf("runInThread(): pid = %d, tid = %d\n", getpid(), gettid());
}

int main() {
//   setloglevel("trace");

  printf("main(): pid = %d, tid = %d\n", getpid(), gettid());

  EventLoopThread loopThread;
  EventLoop* loop = loopThread.startLoop();
  loop->runInLoop(runInThread);
  //   sleep(1);
  loop->runAfter(2000, runInThread);
  sleep(3);
  //   loop->quit();
  printf("exit main().\n");
  //   getchar();
}
