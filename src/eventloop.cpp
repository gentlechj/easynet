#include "eventloop.h"

#include <functional>

#include "channel.h"
#include "logging.h"
#include "poller.h"
namespace easynet {

static thread_local EventLoop *t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

int EventLoop::createWakeupFd() {
  if (::pipe(m_pipeFd) == -1) {
    fatal("Unable to create wakeup fd");
  }
  return m_pipeFd[0];
}

EventLoop::EventLoop()
    : m_looping(false),
      m_threadId(gettid()),
      m_quit(false),
      m_poller(new Poller(this)),
      m_timerManager(new TimerManager(this)),
      m_callingPendingFunctors(false) {
  info("EventLoop created %p in thread %d", this, m_threadId);
  fatalif(t_loopInThisThread, "Another EventLoop %p exists in this thread %d",
          t_loopInThisThread, m_threadId);
  t_loopInThisThread = this;

  int m_wakeupFd = createWakeupFd();
  m_wakeupChannel.reset(new Channel(this, m_wakeupFd));
  m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
  // we are always reading the wakeupfd
  m_wakeupChannel->enableRead();
}

EventLoop::~EventLoop() {
  assert(!m_looping);
  ::close(m_pipeFd[0]);
  ::close(m_pipeFd[1]);
  t_loopInThisThread = nullptr;
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

void EventLoop::abortNotInLoopThread() {
  fatal(
      "EventLoop::abortNotInLoopThread - EventLoop %p was created in "
      "threadId = %d, current thread id = %d",
      this, m_threadId, gettid());
}

TimerId EventLoop::runAt(const TimeStamp time, const TimerCallback &callback) {
  return m_timerManager->addTimer(time, callback);
}

TimerId EventLoop::runAfter(int64_t delay, const TimerCallback &callback) {
  TimeStamp atTime = now() + delay;
  return runAt(atTime, callback);
}

TimerId EventLoop::runEvery(const TimerInterval interval,
                            const TimerCallback &callback) {
  return m_timerManager->addTimer(-1, interval, callback);
}

void EventLoop::runInloop(const Functor &callback) {
  if (isInLoopThread()) {
    callback();
  } else {
    queueInloop(callback);
  }
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = ::write(m_pipeFd[1], &one, sizeof(one));
  if (n != sizeof one) {
    error("EventLoop::wakeup() writes %ld bytes instead of 8", n);
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = ::read(m_wakeupFd, &one, sizeof(one));
  if (n != sizeof one) {
    error("EventLoop::handleRead() reads %ld bytes instead of 8", n);
  }
}

void EventLoop::queueInloop(const Functor &callback) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pendingFunctors.push_back(callback);
  }

  if (!isInLoopThread() || m_callingPendingFunctors) {
    wakeup();
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  m_callingPendingFunctors = true;

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    functors.swap(m_pendingFunctors);
  }

  for (size_t i = 0; i < functors.size(); ++i) {
    // 这里的functor有可能再调用queueInloop
    // 此时需要在queueInloop调用wakeup
    functors[i]();
  }

  m_callingPendingFunctors = false;
}

void EventLoop::loop() {
  assert(!m_looping);
  assertInLoopThread();
  m_looping = true;
  m_quit = false;

  //   ::poll(nullptr, 0, 5 * 1000);

  while (!m_quit) {
    // m_timerManager->checkAndHandleTimers();
    m_activeChannels.clear();
    m_poller->poll(kPollTimeMs, &m_activeChannels);
    for (auto it = m_activeChannels.begin(); it != m_activeChannels.end();
         ++it) {
      (*it)->handleEvent();
    }
    doPendingFunctors();  // handle other things
  }
  info("EventLoop %p stop looping", this);

  m_looping = false;
}

void EventLoop::quit() {
  m_quit = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::updateChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  // 直接调用poller的updateChannel
  // EventLoop不用管poller如何管理channel
  m_poller->updateChannel(channel);
}
}  // namespace easynet