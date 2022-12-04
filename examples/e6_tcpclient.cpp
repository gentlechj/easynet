#include <stdio.h>
#include <unistd.h>

#include <string>
#include <utility>

#include "eventloop.h"
#include "inetaddress.h"
#include "logging.h"
#include "tcpclient.h"

using namespace easynet;
std::string message = "Hello\n";

void onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
    conn->send(message);
  } else {
    printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
  }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf,
               TimeStamp receiveTime) {
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(), conn->name().c_str(),
         readableTime(receiveTime).c_str());

  printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main(int argc, char* argv[]) {
    setloglevel("trace");
  std::string host("localhost");

  if (argc == 2) {
    host = argv[1];
  }

  EventLoop loop;
  Ip4Addr serverAddr(host.c_str(), 3000);
  TcpClient client(&loop, serverAddr);

  client.setConnectionCallback(onConnection);
  client.setMessageCallback(onMessage);
  client.enableRetry();
  client.connect();
  loop.loop();
}