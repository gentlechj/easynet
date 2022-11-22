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

std::string readableTime(time_t t);

int64_t now();
int64_t nowMilliseconds();
int64_t nowMicroseconds();
}  // namespace easynet
