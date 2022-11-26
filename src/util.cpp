#include "util.h"

#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>

#include <memory>

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

int addFdFlag(int fd, int flag) {
  int ret = fcntl(fd, F_GETFD);
  return fcntl(fd, F_SETFD, ret | flag);
}

}  // namespace easynet