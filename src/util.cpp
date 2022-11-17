#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "util.h"
#include <string.h>

namespace easynet {
#ifdef OS_LINUX
    uint64_t gettid() {
        return syscall(SYS_gettid);
    }

#elif defined(OS_MACOSX)

    uint64_t gettid() {
      pthread_t tid = pthread_self();
      uint64_t uid = 0;
      memcpy(&uid, &tid, std::min(sizeof(tid), sizeof(uid)));
      return uid;
    }

#endif

}