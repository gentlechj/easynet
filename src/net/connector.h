#pragma once
#include <functional>
#include <memory>

#include "inetaddress.h"
#include "timer.h"
#include "util.h"

namespace easynet {
class EventLoop;
class Channel;

class Connector : private noncopyable {
 public:
  typedef std::function<void(int sockfd)> NewConnectionCallback;
  Connector(EventLoop* loop, const Ip4Addr& serverAddr);
  ~Connector();

  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    m_newConnectionCallback = cb;
  }

  void start();
  void restart();  // IO线程中调用
  void stop();

  const Ip4Addr& serverAddress() const { return m_serverAddr; }

 private:
  enum StateE { kDisconnected, kConnecting, kConnected };
  static const int kMaxRetryDelayMs;   // 最大重试间隔MS
  static const int kInitRetryDelayMs;  // 初始化重试间隔MS

  void setState(StateE s) { m_state = s; }
  void startInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void handleClose();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

  EventLoop* m_loop;
  Ip4Addr m_serverAddr;
  bool m_connect;
  StateE m_state;
  std::unique_ptr<Channel> m_channel;
  NewConnectionCallback m_newConnectionCallback;
  int m_retryDelayMs;
  TimerId m_timerId;
};

typedef std::shared_ptr<Connector> ConnectorPtr;
}  // namespace easynet