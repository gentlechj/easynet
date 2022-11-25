#pragma once

#include <netinet/in.h>

#include <string>

namespace easynet {

class Ip4Addr {
 public:
  // 根据host和port创建接入点Ï
  Ip4Addr(const std::string& host, unsigned short port);

  // 单独的port，主要用于TCP服务器的监听
  Ip4Addr(unsigned short port = 0) : Ip4Addr("", port) {}

  // 根据传入的struct sockaddr_in创建接入点
  Ip4Addr(const struct sockaddr_in& addr) : m_address(addr){};
  std::string toHostPort() const;

  std::string ip() const;
  unsigned short port() const;
  unsigned int ipInt() const;

  // 检查ip是否有效
  bool isIpValid() const;

  static std::string hostToIp(const std::string& host) {
    Ip4Addr addr(host, 0);
    return addr.ip();
  }

  const struct sockaddr_in& getSockAddrInet() const { return m_address; }
  void setSockAddrInet(const struct sockaddr_in& addr) { m_address = addr; }

 private:
  struct sockaddr_in m_address;
};
}  // namespace easynet