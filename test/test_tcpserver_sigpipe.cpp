#include <stdio.h>

#include "eventLoop.h"
#include "inetaddress.h"
#include "logging.h"
#include "tcpserver.h"

using namespace easynet;

int sleepSeconds = 3;

void onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
    char message[64];
    snprintf(message, sizeof(message) - 1, "TimeStamp: %s",
             readableTime(time(nullptr)).c_str());
    if (sleepSeconds > 0) {
      ::sleep(sleepSeconds);
    }
    conn->send(message);
    if (sleepSeconds > 0) {
      ::sleep(sleepSeconds);
    }
    conn->send(message);
    conn->shutdown();
  }
}

int main() {
  setloglevel("trace");
  printf("main(): pid = %d\n", getpid());

  Ip4Addr listenAddr(3000);
  EventLoop loop;

  TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.start();

  loop.loop();
}