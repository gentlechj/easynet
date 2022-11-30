
#include <stdio.h>

#include "connector.h"
#include "eventLoop.h"

using namespace easynet;
EventLoop* g_loop;

void connectCallback(int sockfd) {
  printf("connected.\n");
  g_loop->quit();
}

int main(int argc, char* argv[]) {
  EventLoop loop;
  g_loop = &loop;
  Ip4Addr addr("127.0.0.1", 3000);
  ConnectorPtr connector(new Connector(&loop, addr));
  connector->setNewConnectionCallback(connectCallback);
  connector->start();

  loop.loop();
}
