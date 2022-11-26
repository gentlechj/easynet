#pragma once

#include <unistd.h>

#include <string>
namespace easynet {
struct noncopyable {
 protected:
  noncopyable() = default;

  virtual ~noncopyable() = default;

  noncopyable(const noncopyable &) = delete;

  noncopyable &operator=(const noncopyable &) = delete;
};

pid_t gettid();

std::string format(const char *fmt, ...);

int addFdFlag(int fd, int flag);
}  // namespace easynet
