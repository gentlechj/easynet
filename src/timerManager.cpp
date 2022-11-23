#include "timerManager.h"

#include "eventloop.h"
#include "logging.h"

namespace easynet {
TimerManager::TimerManager(EventLoop *loop)
    : m_loop(loop), m_timerFdChannel(loop, m_timerFd.getFd()) {
  m_timerFdChannel.setReadCallback(std::bind(&TimerManager::handleRead, this));
  m_timerFdChannel.enableRead();
}

TimerManager::~TimerManager() {
  for (auto it = m_timers.begin(); it != m_timers.end(); ++it) {
    delete *it;
  }
}

void TimerManager::handleRead() {
  m_loop->assertInLoopThread();
  m_timerFd.read();
  checkAndHandleTimers();
  if (!m_timers.empty()) {
    auto nextExpiredTime = (*m_timers.begin())->getExpiredTime();
    m_timerFd.setTime(nextExpiredTime);
  }
}

bool TimerManager::insert(Timer *timer) {
  bool earliestChanged = false;
  TimeStamp when = timer->getExpiredTime();
  auto it = m_timers.begin();
  if (it == m_timers.end() || when < (*it)->getExpiredTime()) {
    earliestChanged = true;
  }
  m_timers.push_back(timer);
  m_timers.sort(TimerCompare());
  return earliestChanged;
}

void TimerManager::addTimerInLoop(Timer *timer) {
  m_loop->assertInLoopThread();
  bool earliestChanged = insert(timer);

  if (earliestChanged) {
    m_timerFd.setTime(timer->getExpiredTime());
  }
}

TimerId TimerManager::addTimer(TimerRepeatedTimes repeatedCount,
                               TimerInterval interval,
                               const TimerCallback &callback) {
  Timer *ptimer = new Timer(repeatedCount, interval, callback);
  m_loop->runInLoop(std::bind(&TimerManager::addTimerInLoop, this, ptimer));
  return ptimer->getId();
}
TimerId TimerManager::addTimer(TimeStamp when, const TimerCallback &callback) {
  Timer *ptimer = new Timer(when, callback);
  m_loop->runInLoop(std::bind(&TimerManager::addTimerInLoop, this, ptimer));
  return ptimer->getId();
}
bool TimerManager::removeTimer(TimerId timerId) {
  for (auto iter = m_timers.begin(); iter != m_timers.end(); ++iter) {
    if ((*iter)->getId() == timerId) {
      Timer *ptimer = *iter;
      m_timers.erase(iter);
      delete ptimer;
      return true;
    }
  }
  return false;
}

void TimerManager::checkAndHandleTimers() {
  // 判断在遍历过程中是否调整了过期时间
  bool adjusted = false;
  Timer *deletedTimer = nullptr;
  for (auto iter = m_timers.begin(); iter != m_timers.end();) {
    if ((*iter)->isExpired()) {
      (*iter)->run();

      if ((*iter)->getRepeatedTimes() == 0) {
        // 移除timer
        deletedTimer = *iter;
        iter = m_timers.erase(iter);
        delete deletedTimer;
        continue;
      } else {
        adjusted = true;
        ++iter;
      }
    } else {
      break;
    }
  }

  if (adjusted) {
    m_timers.sort(TimerCompare());
  }
}
}  // namespace easynet