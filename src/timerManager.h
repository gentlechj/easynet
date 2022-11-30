#pragma once

#include <list>
#include <vector>

#include "channel.h"
#include "timer.h"
#include "timerfd.h"
#include "util.h"

namespace easynet {
class EventLoop;

class TimerManager : private noncopyable {
 public:
  TimerManager(EventLoop* loop);
  ~TimerManager();

  // 添加定时器
  TimerId addTimer(TimerRepeatedTimes repeatedCount, TimerInterval interval,
                   const TimerCallback& callback);

  // 添加定时器
  TimerId addTimer(TimeStamp time, const TimerCallback& callback);

  // 移除指定id的定时器
  void removeTimer(TimerId timerId);

 private:
  struct TimerCompare {
    bool operator()(const Timer* lhs, const Timer* rhs) {
      return lhs->getExpiredTime() < rhs->getExpiredTime();
    }
  };
  struct ActiveTimerCompare {
    bool operator()(const TimerId& lhs, const TimerId& rhs) {
      return lhs < rhs;
    }
  };

  bool insert(Timer* timer);
  void addTimerInLoop(Timer* timer);
  std::vector<Timer*> getExpiredTimers(TimeStamp);
  void checkAndReset(std::vector<Timer*> expiredTimers);
  void removeTimerInLoop(TimerId timerId);

  void handleRead();

 private:
  typedef std::list<Timer*> TimerList;

  // 当前有效的timer，根据timerId排序
  // 应该满足不变式，ActiveTimerList.size() == TimerList.size()
  typedef std::list<TimerId> ActiveTimerList;

  TimerList m_timers;
  ActiveTimerList m_activeTimers;

  TimerFd m_timerFd;
  Channel m_timerFdChannel;
  EventLoop* m_loop;

  // 以下为removeTimer相关
  bool m_callingExpiredTimers;
  // 应对自注销的情况，即在定时器回调中注销当前定时器
  ActiveTimerList m_cancelingTimers;
};
}  // namespace easynet