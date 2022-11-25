#include <stdio.h>

#include "acceptor.h"
#include "eventloop.h"
#include "ip4addr.h"
#include "netutil.h"

using namespace easynet;

void newConnection(int sockfd, const Ip4Addr& peerAddr) {
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(sockfd, "How are you?\n", 13);
  net::close(sockfd);
}

int main() {
  printf("main(): pid = %d\n", getpid());

  Ip4Addr listenAddr(3000);
  EventLoop loop;

  Acceptor acceptor(&loop, listenAddr);
  acceptor.setNewConnectionCallback(newConnection);
  acceptor.listen();

  loop.loop();
}