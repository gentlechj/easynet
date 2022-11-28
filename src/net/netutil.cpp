#include "netutil.h"

#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>

#include "logging.h"
#include "util.h"

namespace easynet {
namespace net {
struct in_addr getHostByName(const std::string &host) {
  struct in_addr addr;
  struct hostent *he = gethostbyname(host.c_str());
  if (he && he->h_addrtype == AF_INET) {
    addr = *reinterpret_cast<struct in_addr *>(he->h_addr);
  } else {
    addr.s_addr = INADDR_NONE;
  }
  return addr;
}

int setNonBlock(int fd, bool on) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) {
    return errno;
  }
  if (on) {
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  }
  return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

int setReuseAddr(int fd, bool on) {
  int flag = on;
  int len = sizeof(flag);
  return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, len);
}

int setReusePort(int fd, bool on) {
#ifndef SO_REUSEPORT
  fatalif(value, "SO_REUSEPORT not supported");
  return 0;
#else
  int flag = on;
  int len = sizeof(flag);
  return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &flag, len);
#endif
}

int setNoDelay(int fd, bool on) {
  int flag = on;
  int len = sizeof(flag);
  return setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &flag, len);
}

void setNonBlockAndCloseOnExec(int sockfd) {
  // non-block
  int ret = setNonBlock(sockfd, true);
  fatalif(ret, "net::setNonBlockAndCloseOnExec setNonBlock failed %d %s", ret,
          strerror(ret));

  // close-on-exec
  ret = addFdFlag(sockfd, FD_CLOEXEC);
  fatalif(ret,
          "net::setNonBlockAndCloseOnExec addFdFlag FD_CLOEXEC failed %d %s",
          ret, strerror(ret));
}

int getSocketError(int sockfd) {
  int optval;
  socklen_t optlen = sizeof optval;

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

struct sockaddr_in getLocalAddr(int sockfd) {
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = sizeof(localaddr);
  if (::getsockname(sockfd, (struct sockaddr *)(&localaddr), &addrlen) < 0) {
    error("sockets::getLocalAddr");
  }
  return localaddr;
}

int createNonBlockSocketFd() {
#ifdef OS_LINUX
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                        IPPROTO_TCP);

#elif defined(OS_MACOSX)
  int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  setNonBlockAndCloseOnExec(sockfd);
#endif
  if (sockfd < 0) {
    fatal("net::createNonblockSocketFd failed");
  }
  return sockfd;
}

void bind(int sockfd, const struct sockaddr_in &addr) {
  int ret = ::bind(sockfd, (struct sockaddr *)(&addr), sizeof(addr));
  if (ret < 0) {
    fatal("net::bind failed");
  }
}
void listen(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    fatal("net::listen failed");
  }
}

int accept(int sockfd, struct sockaddr_in *addr) {
  socklen_t addrlen = sizeof(*addr);
#ifdef OS_LINUX
  int connfd = ::accept4(sockfd, (struct sockaddr *)(addr), &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
#elif defined(OS_MACOSX)
  int connfd = ::accept(sockfd, (struct sockaddr *)(addr), &addrlen);
  setNonBlockAndCloseOnExec(connfd);
#endif
  if (connfd < 0) {
    int savedErrno = errno;
    error("net::accept failed");
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:  // ???
      case EPERM:
      case EMFILE:  // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        fatal("unexpected error of ::accept %d", savedErrno);
        break;
      default:
        fatal("unknown error of ::accept %d", savedErrno);
        break;
    }
  }
  return connfd;
}

void close(int sockfd) {
  if (::close(sockfd) < 0) {
    error("net::close failed");
  }
}

void shutdownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    error("net::shutdownWrite");
  }
}
}  // namespace net
}  // namespace easynet