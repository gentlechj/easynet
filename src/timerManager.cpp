#include "timerManager.h"

#include <algorithm>

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
  TimeStamp now(easynet::now());

  m_timerFd.read();

  std::vector<Timer *> expired = getExpiredTimers(now);

  m_callingExpiredTimers = true;
  m_cancelingTimers.clear();
  for (auto it = expired.begin(); it != expired.end(); ++it) {
    (*it)->run();
  }
  m_callingExpiredTimers = false;

  checkAndReset(expired);
}

bool TimerManager::insert(Timer *timer) {
  bool earliestChanged = false;
  TimeStamp when = timer->getExpiredTime();
  if (m_timers.size() == 0) {
    earliestChanged = true;
  }

  m_timers.push_back(timer);
  m_activeTimers.push_back(timer->getId());
  m_timers.sort(TimerCompare());
  m_activeTimers.sort(ActiveTimerCompare());

  if (when < (*m_timers.begin())->getExpiredTime()) {
    earliestChanged = true;
  }

  assert(m_timers.size() == m_activeTimers.size());
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

void TimerManager::removeTimer(TimerId timerId) {
  m_loop->runInLoop(std::bind(&TimerManager::removeTimerInLoop, this, timerId));
}

void TimerManager::removeTimerInLoop(TimerId timerId) {
  m_loop->assertInLoopThread();
  assert(m_timers.size() == m_activeTimers.size());
  auto findByTimerId = [&](Timer *ptimer) {
    return ptimer->getId() == timerId;
  };

  auto activeTimerIt =
      std::find(m_activeTimers.begin(), m_activeTimers.end(), timerId);

  if (activeTimerIt != m_activeTimers.end()) {
    auto timerIt =
        std::find_if(m_timers.begin(), m_timers.end(), findByTimerId);
    assert(timerIt != m_timers.end());
    Timer *ptimer = *timerIt;
    m_timers.erase(timerIt);
    delete ptimer;
    m_activeTimers.erase(activeTimerIt);
  } else if (m_callingExpiredTimers) {
    m_cancelingTimers.push_back(timerId);
  }
  assert(m_timers.size() == m_activeTimers.size());
}

std::vector<Timer *> TimerManager::getExpiredTimers(TimeStamp now) {
  assert(m_timers.size() == m_activeTimers.size());
  std::vector<Timer *> expiredTimers;
  auto isNotExpired = [&](Timer *ptimer) { return !ptimer->isExpired(now); };
  // 寻找第一个不过期的timer
  auto timerIt = std::find_if(m_timers.begin(), m_timers.end(), isNotExpired);
  assert(timerIt == m_timers.end() || now < (*timerIt)->getExpiredTime());
  std::copy(m_timers.begin(), timerIt, back_inserter(expiredTimers));

  m_timers.erase(m_timers.begin(), timerIt);

  for (auto pTimer : expiredTimers) {
    m_activeTimers.remove(pTimer->getId());
  }

  assert(m_timers.size() == m_activeTimers.size());
  return expiredTimers;
}

void TimerManager::checkAndReset(std::vector<Timer *> expiredTimers) {
  for (auto iter = expiredTimers.begin(); iter != expiredTimers.end(); ++iter) {
    if ((*iter)->getRepeatedTimes() != 0 &&
        std::find(m_cancelingTimers.begin(), m_cancelingTimers.end(),
                  (*iter)->getId()) == m_cancelingTimers.end()) {
      insert(*iter);
    } else {
      delete *iter;
    }
  }

  if (!m_timers.empty()) {
    auto nextExpiredTime = (*m_timers.begin())->getExpiredTime();
    m_timerFd.setTime(nextExpiredTime);
  }
}
}  // namespace easynet