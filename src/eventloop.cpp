#include "eventloop.h"

#include "channel.h"
#include "logging.h"
#include "poller.h"

namespace easynet {

static thread_local EventLoop *t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

EventLoop::EventLoop()
    : m_looping(false),
      m_threadId(gettid()),
      m_quit(false),
      m_poller(new Poller(this)) {
  info("EventLoop created %p in thread %d", this, m_threadId);
  fatalif(t_loopInThisThread, "Another EventLoop %p exists in this thread %d",
          t_loopInThisThread, m_threadId);
  t_loopInThisThread = this;
}

EventLoop::~EventLoop() {
  assert(!m_looping);
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

void EventLoop::loop() {
  assert(!m_looping);
  assertInLoopThread();
  m_looping = true;
  m_quit = false;

  //   ::poll(nullptr, 0, 5 * 1000);

  while (!m_quit) {
    m_activeChannels.clear();
    m_poller->poll(kPollTimeMs, &m_activeChannels);
    for (auto it = m_activeChannels.begin(); it != m_activeChannels.end();
         ++it) {
      (*it)->handleEvent();
    }
  }
  info("EventLoop %p stop looping", this);
  m_looping = false;
}

void EventLoop::quit() { m_quit = true; }

void EventLoop::updateChannel(Channel *channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    // 直接调用poller的updateChannel
    // EventLoop不用管poller如何管理channel
    m_poller->updateChannel(channel);
}
}  // namespace easynet