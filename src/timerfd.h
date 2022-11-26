#pragma once

#include "util.h"
#include "timestamp.h"
namespace easynet {

class TimerFd : private noncopyable {
 public:
  TimerFd();
  ~TimerFd();
  int getFd() const;
  void setTime(TimeStamp timeStamp);
  void alarm();
  void read();

 private:
  int m_pipeFd[2];
  int m_timerFd;
};

}  // namespace easynet
