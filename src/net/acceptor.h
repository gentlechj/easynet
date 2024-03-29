#pragma once
#include <functional>

#include "channel.h"
#include "socket.h"
#include "util.h"
#include "inetaddress.h"

namespace easynet {
class Ip4Addr;
class EventLoop;

class Acceptor : private noncopyable {
 public:
  typedef std::function<void(int sockfd, const Ip4Addr&)> NewConnectionCallback;
  Acceptor(EventLoop* loop, const Ip4Addr& listenAddr);

  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    m_newConnectionCallback = cb;
  }

  bool listening() const { return m_listening; }
  void listen();

 private:
  void handleRead();

  EventLoop* m_loop;
  Ip4Addr m_listenAddr;
  Socket m_acceptSocket;
  Channel m_acceptChannel;
  NewConnectionCallback m_newConnectionCallback;
  bool m_listening;
};
}  // namespace easynet