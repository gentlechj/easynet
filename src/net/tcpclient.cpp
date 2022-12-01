
#include "tcpclient.h"

#include <stdio.h>

#include "connector.h"
#include "eventloop.h"
#include "logging.h"
#include "netutil.h"

namespace easynet {

namespace detail {

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn) {
  loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector) {}

}  // namespace detail

TcpClient::TcpClient(EventLoop* loop, const Ip4Addr& serverAddr)
    : m_loop(loop),
      m_connector(new Connector(loop, serverAddr)),
      m_retry(false),
      m_connect(true),
      m_nextConnId(1) {
  m_connector->setNewConnectionCallback(
      std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
  info("TcpClient::TcpClient[%p] - connector %p", this, m_connector.get());
}

TcpClient::~TcpClient() {
  info("TcpClient::~TcpClient[%p] - connector %p", this, m_connector.get());

  TcpConnectionPtr conn;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    conn = m_connection;
  }

  if (conn) {
    // client已经销毁，将removeConnection的工作转到detail::removeConnection
    CloseCallback cb =
        std::bind(&detail::removeConnection, m_loop, std::placeholders::_1);
    m_loop->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
  } else {
    m_connector->stop();
    m_loop->runAfter(1000, std::bind(&detail::removeConnector, m_connector));
  }
}

void TcpClient::connect() {
  info("TcpClient::connect[%p] - connecting to %s", this,
       m_connector->serverAddress().toHostPort().c_str());
  m_connect = true;
  m_connector->start();
}

void TcpClient::disconnect() {
  m_connect = false;

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_connection) {
      m_connection->shutdown();
    }
  }
}

void TcpClient::stop() {
  m_connect = false;
  m_connector->stop();
}

void TcpClient::newConnection(int sockfd) {
  m_loop->assertInLoopThread();
  Ip4Addr peerAddr(net::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toHostPort().c_str(),
           m_nextConnId);
  ++m_nextConnId;
  std::string connName = buf;

  Ip4Addr localAddr(net::getLocalAddr(sockfd));

  TcpConnectionPtr conn(
      new TcpConnection(m_loop, connName, sockfd, localAddr, peerAddr));

  conn->setConnectionCallback(m_connectionCallback);
  conn->setMessageCallback(m_messageCallback);
  conn->setWriteCompleteCallback(m_writeCompleteCallback);
  conn->setCloseCallback(
      std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connection = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
  m_loop->assertInLoopThread();
  assert(m_loop == conn->getLoop());
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    assert(m_connection == conn);
    m_connection.reset();
  }

  m_loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  if (m_retry && m_connect) {
    info("TcpClient::connect[%p] - Reconnecting to %s", this,
         m_connector->serverAddress().toHostPort().c_str());
    m_connector->restart();
  }
}
}  // namespace easynet