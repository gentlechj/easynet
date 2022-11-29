#pragma once
#include <memory>

#include "buffer.h"
#include "callback.h"
#include "inetaddress.h"
#include "timestamp.h"
#include "util.h"
namespace easynet {
class Channel;
class EventLoop;
class Socket;

class TcpConnection : private noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                const Ip4Addr& localAddr, const Ip4Addr& peerAddr);
  ~TcpConnection();
  bool connected() const { return m_state == kConnected; }

  const Ip4Addr& peerAddress() const { return m_peerAddr; }
  const Ip4Addr& localAddress() const { return m_localAddr; }

  void setConnectionCallback(const ConnectionCallback& cb) {
    m_connectionCallback = cb;
  }
  void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }

  void setCloseCallback(const CloseCallback& cb) { m_closeCallback = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    m_writeCompleteCallback = cb;
  }

  std::string name() { return m_name; }

  // 当accept一个新连接的时候调用
  void connectEstablished();
  // 当被移除的时候调用
  void connectDestroyed();

  void send(const std::string& message);
  void shutdown();
  void setTcpNoDelay(bool on);

  EventLoop* getLoop() { return m_loop; }

 private:
  // 没有发起连接的功能，初始状态是kConnecting
  enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };

  void setState(StateE state) { m_state = state; }
  void handleRead(TimeStamp);
  void handleWrite();
  void handleClose();
  void handleError();
  void sendInLoop(const std::string& message);
  void shutdownInLoop();

 private:
  EventLoop* m_loop;
  std::string m_name;
  StateE m_state;

  std::unique_ptr<Channel> m_channel;
  std::unique_ptr<Socket> m_socket;
  Ip4Addr m_localAddr, m_peerAddr;
  ConnectionCallback m_connectionCallback;
  MessageCallback m_messageCallback;
  CloseCallback m_closeCallback;
  WriteCompleteCallback m_writeCompleteCallback;

  Buffer m_inputBuffer;
  Buffer m_outputBuffer;
};

}  // namespace easynet