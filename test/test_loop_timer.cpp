#include <stdio.h>

#include <functional>

#include "eventLoop.h"
#include "util.h"

using namespace easynet;

int cnt = 0;
EventLoop* g_loop;

void printTid() {
  printf("pid = %d, tid = %d\n", getpid(), gettid());
  printf("now %ld\n", time(nullptr));
}

void print(const char* msg) {
  printf("msg %ld %s\n", time(nullptr), msg);
  if (++cnt == 20) {
    g_loop->quit();
  }
}

int main() {
  printTid();
  EventLoop loop;
  g_loop = &loop;

  print("main");
  loop.runAfter(1000, std::bind(print, "once1"));
  loop.runAfter(1000, std::bind(print, "once2"));
  loop.runAfter(3000, std::bind(print, "once3"));
  loop.runAfter(4000, std::bind(print, "once4"));
  loop.runEvery(2000, std::bind(print, "every2"));
  loop.runEvery(3000, std::bind(print, "every3"));

  loop.loop();
  print("main loop exits");
  sleep(1);
}