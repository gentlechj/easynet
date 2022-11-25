#include "acceptor.h"

#include "eventloop.h"
#include "logging.h"
#include "netutil.h"

namespace easynet {

Acceptor::Acceptor(EventLoop* loop, const Ip4Addr& listenAddr)
    : m_loop(loop),
      m_listenAddr(listenAddr),
      m_acceptSocket(net::createNonBlockSocketFd()),
      m_acceptChannel(loop, m_acceptSocket.fd()),
      m_listening(false) {
  m_acceptSocket.setReuseAddr(true);
  m_acceptSocket.bindAddress(listenAddr);
  m_acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen() {
  m_loop->assertInLoopThread();
  m_listening = true;
  m_acceptSocket.listen();
  m_acceptChannel.enableRead();
  info("listening on %s", m_listenAddr.toHostPort().c_str());
}

void Acceptor::handleRead() {
  m_loop->assertInLoopThread();
  Ip4Addr peerAddr(0);

  // TODO 循环accept，直到没有新的连接
  int connfd = m_acceptSocket.accept(&peerAddr);
  if (connfd >= 0) {
    if (m_newConnectionCallback) {
      m_newConnectionCallback(connfd, peerAddr);
    } else {
      net::close(connfd);
    }
  }
}

}  // namespace easynet