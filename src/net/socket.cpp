
#include "socket.h"

#include <string.h>

#include "inetaddress.h"
#include "netutil.h"

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

void Socket::setReuseAddr(bool on) { net::setReuseAddr(m_socketfd, on); }
void Socket::setTcpNoDelay(bool on) { net::setTcpNoDelay(m_socketfd, on); }
void Socket::setReusePort(bool on) { net::setReusePort(m_socketfd, on); }
void Socket::setNonBlock(bool on) { net::setNonBlock(m_socketfd, on); }

void Socket::shutdownWrite() { net::shutdownWrite(m_socketfd); }
}  // namespace easynet