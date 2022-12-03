#include <stdio.h>

#include "eventLoop.h"
#include "inetaddress.h"
#include "tcpserver.h"
#include "logging.h"

using namespace easynet;
void onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
  } else {
    printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
  }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf,
               TimeStamp receiveTime) {
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(), conn->name().c_str(),
         readableTime(receiveTime).c_str());

  conn->send(buf->retrieveAsString());
}

int main(int argc, char* argv[]) {
  setloglevel("trace");
  printf("main(): pid = %d\n", getpid());

  Ip4Addr listenAddr(3000);
  EventLoop loop;

  TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  }
  server.start();

  loop.loop();
}