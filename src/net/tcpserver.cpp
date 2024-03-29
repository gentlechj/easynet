#include "tcpserver.h"

#include "acceptor.h"
#include "eventloop.h"
#include "eventloopthreadpool.h"
#include "logging.h"
#include "netutil.h"
namespace easynet {
TcpServer::TcpServer(EventLoop* loop, const Ip4Addr& listenAddr)
    : m_loop(loop),
      m_name(listenAddr.toHostPort()),
      m_acceptor(new Acceptor(loop, listenAddr)),
      m_threadPool(new EventLoopThreadPool(loop)),
      m_started(false),
      m_nextConnId(1) {
  m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection,
                                                 this, std::placeholders::_1,
                                                 std::placeholders::_2));
}

TcpServer::~TcpServer() {}

void TcpServer::setThreadNum(int numThreads) {
  assert(0 <= numThreads);
  m_threadPool->setThreadNum(numThreads);
}

void TcpServer::start() {
  if (!m_started) {
    m_started = true;
    m_threadPool->start();
  }

  if (!m_acceptor->listening()) {
    m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
  }
}

void TcpServer::newConnection(int sockfd, const Ip4Addr& peerAddr) {
  m_loop->assertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof buf, "#%d", m_nextConnId);
  ++m_nextConnId;
  std::string connName = m_name + buf;

  info("TcpServer::newConnection [\"%s\"] - new connection [\"%s\"] from %s",
       m_name.c_str(), connName.c_str(), peerAddr.toHostPort().c_str());
  Ip4Addr localAddr(net::getLocalAddr(sockfd));

  EventLoop* ioLoop = m_threadPool->getNextLoop();

  TcpConnectionPtr conn(
      new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
  m_connections[connName] = conn;
  conn->setConnectionCallback(m_connectionCallback);
  conn->setMessageCallback(m_messageCallback);
  conn->setWriteCompleteCallback(m_writeCompleteCallback);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
  // TODO: use ctor arguments
  //   conn->setTcpNoDelay(true);
  //   conn->connectEstablished();
  // 在IO线程执行connectEstablished回调
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  m_loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
  m_loop->assertInLoopThread();
  info("TcpServer::removeConnection [\"%s\"] - connection ",
       conn->name().c_str());

  size_t n = m_connections.erase(conn->name());
  assert(n == 1);

  EventLoop* ioLoop = conn->getLoop();
  // std::bind延长TcpConnection的生命周期
  // 避免channel在handle event的时候被析构，造成崩溃
  // 不在channel处理close event的时候析构

  ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

}  // namespace easynet