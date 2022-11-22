#pragma once

#include <sys/time.h>

#include "util.h"
namespace easynet {

class TimerFd : private noncopyable {
 public:
  TimerFd();
  ~TimerFd();
  int getFd() const;
  void setTime(int64_t timeStamp);
  void alarm();
  void read();

 private:
  int m_pipeFd[2];
  int m_timerFd;
};

}  // namespace easynet
