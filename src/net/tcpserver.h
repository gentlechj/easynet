#pragma once
#include <map>

#include "callback.h"
#include "inetaddress.h"
#include "tcpconnection.h"
#include "util.h"

namespace easynet {
class Acceptor;
class EventLoop;

class TcpServer : private noncopyable {
 public:
  TcpServer(EventLoop* loop, const Ip4Addr& listenAddr);
  ~TcpServer();
  void start();

  void setConnectionCallback(const ConnectionCallback& cb) {
    m_connectionCallback = cb;
  }

  void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    m_writeCompleteCallback = cb;
  }

 private:
  void newConnection(int sockfd, const Ip4Addr& peerAddr);
  void removeConnection(const TcpConnectionPtr& conn);

  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop* m_loop;
  const std::string m_name;
  std::unique_ptr<Acceptor> m_acceptor;
  ConnectionCallback m_connectionCallback;
  MessageCallback m_messageCallback;
  WriteCompleteCallback m_writeCompleteCallback;
  bool m_started;
  int m_nextConnId;
  ConnectionMap m_connections;
};
}  // namespace easynet