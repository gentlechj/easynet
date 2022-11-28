#pragma once
#include "util.h"

namespace easynet {
class Ip4Addr;
class Socket : private noncopyable {
 public:
  explicit Socket(int sockfd) : m_socketfd(sockfd) {}
  ~Socket();

  void bindAddress(const Ip4Addr& localaddr);
  void listen();

  // 成功返回accepted socket描述符（non-blocking and close-on-exec）
  // 失败返回-1
  int accept(Ip4Addr* peeraddr);

  void setReuseAddr(bool on = true);
  void setNonBlock(bool on = true);
  void setReusePort(bool on = true);
  void setNoDelay(bool on = true);
  void shutdownWrite();

  int fd() const { return m_socketfd; }

 private:
  const int m_socketfd;
};
}  // namespace easynet