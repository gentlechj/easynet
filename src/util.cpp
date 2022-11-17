#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "util.h"
#include <string.h>

namespace easynet {
#ifdef OS_LINUX
    pid_t gettid() {
        return syscall(SYS_gettid);
    }

#elif defined(OS_MACOSX)

    pid_t gettid() {
      pthread_t tid = pthread_self();
      pid_t uid = 0;
      memcpy(&uid, &tid, std::min(sizeof(tid), sizeof(uid)));
      return uid;
    }

#endif

}