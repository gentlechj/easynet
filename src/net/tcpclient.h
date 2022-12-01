#pragma once
#include <memory>
#include <mutex>

#include "tcpconnection.h"
#include "util.h"

namespace easynet {

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : private noncopyable {
 public:
  TcpClient(EventLoop* loop, const Ip4Addr& serverAddr);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connection;
  }

  bool retry() const;
  void enableRetry() { m_retry = true; }

  void setConnectionCallback(const ConnectionCallback& cb) {
    m_connectionCallback = cb;
  }
  void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    m_writeCompleteCallback = cb;
  }

 private:
  void newConnection(int sockfd);
  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop* m_loop;
  ConnectorPtr m_connector;

  // 各种回调
  ConnectionCallback m_connectionCallback;
  MessageCallback m_messageCallback;
  WriteCompleteCallback m_writeCompleteCallback;

  // 状态相关
  bool m_retry;
  bool m_connect;

  int m_nextConnId;
  mutable std::mutex m_mutex;
  TcpConnectionPtr m_connection;
};

}  // namespace easynet