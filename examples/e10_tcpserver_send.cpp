#include <stdio.h>

#include "eventloop.h"
#include "inetaddress.h"
#include "logging.h"
#include "tcpserver.h"

using namespace easynet;
std::string message1;
std::string message2;
int sleepSeconds = 0;

void onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
    if (sleepSeconds > 0) {
      ::sleep(sleepSeconds);
    }
    conn->send(message1);
    conn->send(message2);
    conn->shutdown();
  } else {
    printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
  }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf,
               TimeStamp receiveTime) {
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(), conn->name().c_str(),
         readableTime(receiveTime).c_str());

  buf->retrieveAll();
}

int main(int argc, char* argv[]) {
  printf("main(): pid = %d\n", getpid());

  int len1 = 100;
  int len2 = 200;

  if (argc > 2) {
    len1 = atoi(argv[1]);
    len2 = atoi(argv[2]);
  }
  if (argc > 3) {
    sleepSeconds = atoi(argv[3]);
  }

  message1.resize(len1);
  message2.resize(len2);
  std::fill(message1.begin(), message1.end(), 'A');
  std::fill(message2.begin(), message2.end(), 'B');

  Ip4Addr listenAddr(3000);
  EventLoop loop;

  TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  if (argc > 3) {
    server.setThreadNum(atoi(argv[3]));
  }
  server.start();

  loop.loop();
}