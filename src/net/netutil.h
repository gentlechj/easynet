#pragma once

#include <netinet/in.h>

#include <string>

namespace easynet {
namespace net {
static const int kLittleEndian = LITTLE_ENDIAN;
inline uint16_t htobe(uint16_t v) {
  if (!kLittleEndian) {
    return v;
  }
  unsigned char *pv = (unsigned char *)&v;
  return uint16_t(pv[0]) << 8 | uint16_t(pv[1]);
}
inline uint32_t htobe(uint32_t v) {
  if (!kLittleEndian) {
    return v;
  }
  unsigned char *pv = (unsigned char *)&v;
  return uint32_t(pv[0]) << 24 | uint32_t(pv[1]) << 16 | uint32_t(pv[2]) << 8 |
         uint32_t(pv[3]);
}
inline uint64_t htobe(uint64_t v) {
  if (!kLittleEndian) {
    return v;
  }
  unsigned char *pv = (unsigned char *)&v;
  return uint64_t(pv[0]) << 56 | uint64_t(pv[1]) << 48 | uint64_t(pv[2]) << 40 |
         uint64_t(pv[3]) << 32 | uint64_t(pv[4]) << 24 | uint64_t(pv[5]) << 16 |
         uint64_t(pv[6]) << 8 | uint64_t(pv[7]);
}
inline int16_t htobe(int16_t v) { return (int16_t)htobe((uint16_t)v); }
inline int32_t htobe(int32_t v) { return (int32_t)htobe((uint32_t)v); }
inline int64_t htobe(int64_t v) { return (int64_t)htobe((uint64_t)v); }

struct in_addr getHostByName(const std::string &host);

// 设置socket fd相关属性
int setNonBlock(int fd, bool on = true);
int setCloExec(int fd, bool on = true);
int setReuseAddr(int fd, bool on = true);
int setReusePort(int fd, bool on = true);
int setTcpNoDelay(int fd, bool on = true);

// 获取socket fd相关属性
int getSocketError(int sockfd);
struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);

// socket相关操作
int createNonBlockSocketFd();
void bind(int sockfd, const struct sockaddr_in &addr);
void listen(int sockfd);
int accept(int sockfd, struct sockaddr_in *addr);
void close(int sockfd);
void shutdownWrite(int sockfd);
int connect(int sockfd, const struct sockaddr_in &addr);
}  // namespace net
}  // namespace easynet