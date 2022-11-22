#include "util.h"

#include <pthread.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>

#include <chrono>
#include <memory>
#include <string>
namespace easynet {
#ifdef OS_LINUX
pid_t gettid() { return syscall(SYS_gettid); }

#elif defined(OS_MACOSX)

pid_t gettid() {
  pthread_t tid = pthread_self();
  pid_t uid = 0;
  memcpy(&uid, &tid, std::min(sizeof(tid), sizeof(uid)));
  return uid;
}

#endif

std::string format(const char *fmt, ...) {
  char buffer[500];
  std::unique_ptr<char[]> release1;
  char *base;
  for (int iter = 0; iter < 2; iter++) {
    int bufsize;
    if (iter == 0) {
      bufsize = sizeof(buffer);
      base = buffer;
    } else {
      bufsize = 30000;
      base = new char[bufsize];
      release1.reset(base);
    }
    char *p = base;
    char *limit = base + bufsize;
    if (p < limit) {
      va_list ap;
      va_start(ap, fmt);
      p += vsnprintf(p, limit - p, fmt, ap);
      va_end(ap);
    }
    // Truncate to available space if necessary
    if (p >= limit) {
      if (iter == 0) {
        continue;  // Try again with larger buffer
      } else {
        p = limit - 1;
        *p = '\0';
      }
    }
    break;
  }
  return base;
}

std::string readableTime(time_t t) {
  struct tm tm1;
  localtime_r(&t, &tm1);
  return format("%04d-%02d-%02d %02d:%02d:%02d", tm1.tm_year + 1900,
                tm1.tm_mon + 1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min,
                tm1.tm_sec);
}

int64_t now() { return nowMilliseconds(); }
int64_t nowMilliseconds() {
  std::chrono::time_point<std::chrono::system_clock> p =
      std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             p.time_since_epoch())
      .count();
}
int64_t nowMicroseconds() {
  std::chrono::time_point<std::chrono::system_clock> p =
      std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(
             p.time_since_epoch())
      .count();
}
}  // namespace easynet