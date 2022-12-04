#pragma once

#include <functional>
#include <mutex>

#include "timestamp.h"
namespace easynet {
typedef int64_t TimerId;
typedef int32_t TimerRepeatedTimes;
typedef int64_t TimerInterval;

typedef std::function<void()> TimerCallback;

class Timer {
 public:
  Timer(TimerRepeatedTimes repeatedTimes, TimerInterval interval,
        const TimerCallback& timercallback);

  Timer(TimeStamp time, const TimerCallback& timercallback);
  ~Timer() = default;

  bool isExpired(TimeStamp now) const;
  void run();

  TimerId getId() const { return m_id; }
  TimerRepeatedTimes getRepeatedTimes() { return m_repeatedTimes; }
  TimeStamp getExpiredTime() const { return m_expiredTime; }

 public:
  static TimerId generateId();

 private:
  TimerId m_id;                        // 定时器id
  TimerRepeatedTimes m_repeatedTimes;  // 定时器重复触发的次数
  TimerInterval m_interval;            // 触发时间间隔

  TimeStamp m_expiredTime;   // 定时器到期时间
  TimerCallback m_callback;  // 定时器触发后的回调函数

  static TimerId s_initialId;  // 定时器ID值
  static std::mutex s_mutex;   // 保护s_initialId的互斥体对象
};
}  // namespace easynet
