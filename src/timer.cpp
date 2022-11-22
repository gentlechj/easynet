
#include "timer.h"

#include <time.h>

#include "util.h"

namespace easynet {

TimerId Timer::s_initialId{0};
std::mutex Timer::s_mutex{};

TimerId Timer::generateId() {
  TimerId tmpId;
  s_mutex.lock();
  ++s_initialId;
  tmpId = s_initialId;
  s_mutex.unlock();
  return tmpId;
}

Timer::Timer(TimerRepeatedTimes repeatedTimes, TimerInterval interval,
             const TimerCallback& timercallback)
    : m_repeatedTimes(repeatedTimes),
      m_interval(interval),
      m_callback(timercallback) {
  // 当前时间加上触发间隔等于下一次的过期时间
  m_expiredTime = now() + interval;

  // 生成唯一id
  m_id = Timer::generateId();
}

Timer::Timer(TimeStamp when, const TimerCallback& timercallback)
    : m_repeatedTimes(0), m_callback(timercallback), m_interval(0) {
  m_expiredTime = when;

  // 生成唯一id
  m_id = Timer::generateId();
}

bool Timer::isExpired() const { return now() >= m_expiredTime; }

void Timer::run() {
  m_callback();
  if (m_repeatedTimes >= 1) {
    --m_repeatedTimes;
  }
  m_expiredTime += m_interval;
}
}  // namespace easynet