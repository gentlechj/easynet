#pragma once
#include <functional>

#include "channel.h"
#include "socket.h"
#include "util.h"
#include "Ip4Addr.h"

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

  bool listenging() const { return m_listening; }
  void listen();

 private:
  void handleRead();

  EventLoop* m_loop;
  Socket m_acceptSocket;
  Channel m_acceptChannel;
  NewConnectionCallback m_newConnectionCallback;
  bool m_listening;
  Ip4Addr m_listenAddr;
};
}  // namespace easynet