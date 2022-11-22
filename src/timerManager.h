#pragma once

#include <list>

#include "channel.h"
#include "timer.h"
#include "timerfd.h"
#include "util.h"

namespace easynet {
class EventLoop;

struct TimerCompare {
  bool operator()(const Timer* lhs, const Timer* rhs) {
    return lhs->getExpiredTime() < rhs->getExpiredTime();
  }
};

class TimerManager : private noncopyable {
 public:
  typedef std::list<Timer*> TimerList;
  TimerManager(EventLoop* loop);
  ~TimerManager();

  // 添加定时器
  TimerId addTimer(TimerRepeatedTimes repeatedCount, TimerInterval interval,
                   const TimerCallback& callback);

  // 添加定时器
  TimerId addTimer(TimeStamp time, const TimerCallback& callback);

  // 移除指定id的定时器
  bool removeTimer(TimerId id);

 private:
  bool insert(Timer* timer);
  void addTimerInLoop(Timer* timer);
  void checkAndHandleTimers();

  void handleRead();

 private:
  TimerList m_timers;
  TimerFd m_timerFd;
  Channel m_timerFdChannel;
  EventLoop* m_loop;
};
}  // namespace easynet