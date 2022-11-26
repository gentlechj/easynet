
#include "tcpconnection.h"

#include <errno.h>
#include <string.h>

#include "channel.h"
#include "eventloop.h"
#include "logging.h"
#include "netutil.h"
#include "socket.h"

namespace easynet {
TcpConnection::TcpConnection(EventLoop* loop, const std::string& nameArg,
                             int sockfd, const Ip4Addr& localAddr,
                             const Ip4Addr& peerAddr)
    : m_loop(loop),
      m_name(nameArg),
      m_state(kConnecting),
      m_socket(new Socket(sockfd)),
      m_channel(new Channel(loop, sockfd)),
      m_localAddr(localAddr),
      m_peerAddr(peerAddr) {
  trace("TcpConnection::ctor[\"%s\"] at %p\" fd= %d", m_name.c_str(), this,
        sockfd);
  m_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this));
  m_channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  m_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  m_channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
  trace("TcpConnection::dtor[\"%s\"] at %p\" fd= %d", m_name.c_str(), this,
        m_channel->fd());
}

void TcpConnection::connectEstablished() {
  m_loop->assertInLoopThread();
  assert(m_state == kConnecting);
  setState(kConnected);
  m_channel->enableRead();
  m_connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed() {
  m_loop->assertInLoopThread();
  assert(m_state == kConnected);
  setState(kDisconnected);
  m_channel->disableAll();
  m_connectionCallback(shared_from_this());
  m_loop->removeChannel(m_channel.get());
}

void TcpConnection::handleRead() {
  char buf[65535];
  ssize_t n = ::read(m_channel->fd(), buf, sizeof(buf));
  if (n > 0) {
    m_messageCallback(shared_from_this(), buf, n);
  } else if (n == 0) {
    handleClose();
  } else {
    handleError();
  }
}
void TcpConnection::handleWrite() {
  // TODO
}

void TcpConnection::handleClose() {
  m_loop->assertInLoopThread();
  trace("TcpConnection::handleClose state = %d", m_state);
  assert(m_state == kConnected);
  // 不关闭fd，使用析构函数进行关闭，RAII
  m_channel->disableAll();
  m_closeCallback(shared_from_this());
}

void TcpConnection::handleError() {
  int err = net::getSocketError(m_channel->fd());
  error("TcpConnection::handleError [%s] - SO_ERROR = %d %s", m_name.c_str(),
        err, strerror(err));
}

}  // namespace easynet