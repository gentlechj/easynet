
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
  m_channel->setReadCallback(
      std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
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
  assert(m_state == kConnected || m_state == kDisconnecting);
  setState(kDisconnected);
  m_channel->disableAll();
  m_connectionCallback(shared_from_this());
  m_loop->removeChannel(m_channel.get());
}

void TcpConnection::handleRead(TimeStamp receiveTime) {
  //   char buf[65535];
  //   ssize_t n = ::read(m_channel->fd(), buf, sizeof(buf));
  int savedErrno = 0;
  ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &savedErrno);
  if (n > 0) {
    m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = savedErrno;
    error("TcpConnection::handleRead %d", savedErrno);
    handleError();
  }
}
void TcpConnection::handleWrite() {
  m_loop->assertInLoopThread();
  if (m_channel->isWriting()) {
    // TODO use writev?
    ssize_t n = ::write(m_channel->fd(), m_outputBuffer.peek(),
                        m_outputBuffer.readableBytes());
    if (n > 0) {
      m_outputBuffer.retrieve(n);
      if (m_outputBuffer.readableBytes() == 0) {
        // 停止观察writable事件
        m_channel->disableWrite();
        // 当连接正在关闭的时候，继续执行关闭过程
        if (m_state == kDisconnecting) {
          shutdownInLoop();
        }
      } else {
        trace("Pending to write more data");
      }
    } else {
      error("TcpConnection::handleWrite");
    }
  } else {
    trace("TcpConnection is down, no more writing");
  }
}

void TcpConnection::handleClose() {
  m_loop->assertInLoopThread();
  trace("TcpConnection::handleClose state = %d", m_state);
  assert(m_state == kConnected || m_state == kDisconnecting);
  // 不关闭fd，使用析构函数进行关闭，RAII
  m_channel->disableAll();
  m_closeCallback(shared_from_this());
}

void TcpConnection::handleError() {
  int err = net::getSocketError(m_channel->fd());
  error("TcpConnection::handleError [%s] - SO_ERROR = %d %s", m_name.c_str(),
        err, strerror(err));
}

void TcpConnection::shutdown() {
  if (m_state == kConnected) {
    setState(kDisconnecting);
    m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop() {
  m_loop->assertInLoopThread();
  if (!m_channel->isWriting()) {
    m_socket->shutdownWrite();
  }
}

void TcpConnection::send(const std::string& message) {
  if (m_state == kConnected) {
    if (m_loop->isInLoopThread()) {
      sendInLoop(message);
    } else {
      m_loop->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
    }
  }
}

void TcpConnection::sendInLoop(const std::string& message) {
  m_loop->assertInLoopThread();
  ssize_t nWrote = 0;
  if (!m_channel->isWriting() && m_outputBuffer.readableBytes() == 0) {
    nWrote = ::write(m_channel->fd(), message.data(), message.size());
    if (nWrote > 0) {
      if (static_cast<size_t>(nWrote) < message.size()) {
        trace("Pending to write more data");
      }
    } else {
      nWrote = 0;
      if (errno != EWOULDBLOCK) {
        error("TcpConnection::sendInLoop");
      }
    }
  }

  assert(nWrote >= 0);
  if (static_cast<size_t>(nWrote) < message.size()) {
    m_outputBuffer.append(message.data() + nWrote, message.size() - nWrote);
    // 观察writable事件，然后发送数据
    if (!m_channel->isWriting()) {
      m_channel->enableWrite();
    }
  }
}
}  // namespace easynet