#include "connector.h"

#include <errno.h>
#include <string.h>

#include "eventloop.h"
#include "logging.h"
#include "netutil.h"

namespace easynet {
const int Connector::kMaxRetryDelayMs = 30 * 1000;  // 最大重试间隔MS
const int Connector::kInitRetryDelayMs = 500;       // 初始化重试间隔MS

Connector::Connector(EventLoop* loop, const Ip4Addr& serverAddr)
    : m_loop(loop),
      m_serverAddr(serverAddr),
      m_connect(false),
      m_state(kDisconnected),
      m_retryDelayMs(kInitRetryDelayMs) {
  trace("Connector ctor[%p]", this);
}
Connector::~Connector() {
  trace("Connector dtor[%p]", this);
  // TODO 处理定时器未过期的时候，connetor被销毁的情况，取消定时器
  //   m_loop->cancel(timerId_);
  assert(!m_channel);
}

void Connector::start() {
  m_connect = true;
  m_loop->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop() {
  m_loop->assertInLoopThread();
  assert(m_state == kDisconnected);
  if (m_connect) {
    connect();
  } else {
    trace("Connector[%p] do not connect", this);
  }
}

void Connector::connect() {
  int sockfd = net::createNonBlockSocketFd();
  int ret = net::connect(sockfd, m_serverAddr.getSockAddrInet());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno) {
    case 0:
    // 连接中
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;
    // 连接出错
    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      error("connect error in Connector::startInLoop %d", savedErrno);
      net::close(sockfd);
      break;

    default:
      error("Unexpected error in Connector::startInLoop %d", savedErrno);
      net::close(sockfd);
      break;
  }
}

void Connector::restart() {
  m_loop->assertInLoopThread();
  setState(kDisconnected);
  m_retryDelayMs = kInitRetryDelayMs;
  m_connect = true;
  startInLoop();
}

void Connector::stop() {
  m_connect = false;
  // TODO 处理定时器未过期的时候，connetor被销毁的情况，取消定时器
  //   m_loop->cancel(timerId_);
}

void Connector::connecting(int sockfd) {
  setState(kConnecting);
  assert(!m_channel);
  m_channel.reset(new Channel(m_loop, sockfd));
  m_channel->setWriteCallback(std::bind(&Connector::handleWrite, this));
  m_channel->setErrorCallback(std::bind(&Connector::handleError, this));
  m_channel->enableWrite();
}

int Connector::removeAndResetChannel() {
  m_channel->disableAll();
  m_loop->removeChannel(m_channel.get());
  int sockfd = m_channel->fd();
  // 此处不能重置channel，因为还处于Channel::handleEvent中
  m_loop->queueInLoop(std::bind(&Connector::resetChannel, this));
  return sockfd;
}

void Connector::resetChannel() { m_channel.reset(); }

void Connector::handleWrite() {
  trace("Connector::handleWrite %d", m_state);

  // Connector可写，说明连接建立完成
  if (m_state == kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = net::getSocketError(sockfd);
    if (err) {
      warn("Connector::handleWrite - SO_ERROR = %d %s", err, strerror(err));
      retry(sockfd);
    }
    // 自连接的情况
    else if (net::isSelfConnect(sockfd)) {
      warn("Connector::handleWrite - Self connect");
      retry(sockfd);
    } else {
      setState(kConnected);
      if (m_connect) {
        m_newConnectionCallback(sockfd);
      } else {
        net::close(sockfd);
      }
    }
  } else {
    assert(m_state == kDisconnected);
  }
}

void Connector::handleError() {
  error("Connector::handleError");
  assert(m_state == kConnecting);

  // 出错重连
  // 首先移除channel，关闭socket fd, 避免socket fd泄漏
  int sockfd = removeAndResetChannel();
  int err = net::getSocketError(sockfd);
  trace("SO_ERROR = %d %s", err, strerror(err));
  retry(sockfd);
}

void Connector::retry(int sockfd) {
  net::close(sockfd);
  setState(kDisconnected);
  if (m_connect) {
    info("Connector::retry - Retry connecting to %s in %d milliseconds.",
         m_serverAddr.toHostPort().c_str(), m_retryDelayMs);
    m_timerId = m_loop->runAfter(m_retryDelayMs,
                                 std::bind(&Connector::startInLoop, this));
    // 每次重连间隔为当前的两倍
    m_retryDelayMs = std::min(m_retryDelayMs * 2, kMaxRetryDelayMs);
  } else {
    trace("Connector[%p] do not connect", this);
  }
}
}  // namespace easynet