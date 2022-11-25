
#include "socket.h"

#include "Ip4Addr.h"
#include "netutil.h"
#include <string.h>

namespace easynet {
Socket::~Socket() { net::close(m_socketfd); }
void Socket::bindAddress(const Ip4Addr& addr) {
  net::bind(m_socketfd, addr.getSockAddrInet());
}

void Socket::listen() { net::listen(m_socketfd); }

int Socket::accept(Ip4Addr* peeraddr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  int connfd = net::accept(m_socketfd, &addr);
  if (connfd >= 0) {
    peeraddr->setSockAddrInet(addr);
  }
  return connfd;
}

void Socket::setReuseAddr(bool on) { net::setReuseAddr(m_socketfd); }
}  // namespace easynet