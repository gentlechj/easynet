#include "epollpoller.h"

#include <poll.h>
#include <string.h>

#include "channel.h"
#include "logging.h"

#ifdef OS_LINUX
#include <sys/epoll.h>
#elif defined(OS_MACOSX)
#include <sys/event.h>
#else
#error "platform unsupported"
#endif

namespace easynet {
namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}  // namespace

#ifdef OS_MACOSX
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

EPollPoller::EPollPoller(EventLoop* loop)
    : m_loop(loop), m_epoll_fd(::kqueue()), m_events(kInitEventListSize) {
  if (m_epoll_fd < 0) {
    fatal("EPollPoller::EPollPoller");
  }
}

EPollPoller::~EPollPoller() { ::close(m_epoll_fd); }

void EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
  struct timespec timeout;
  timeout.tv_sec = timeoutMs / 1000;
  timeout.tv_nsec = (timeoutMs % 1000) * 1000 * 1000;

  int numEvents = ::kevent(m_epoll_fd, NULL, 0, m_events.data(),
                           static_cast<int>(m_events.size()), &timeout);
  long ticks = easynet::now();

  trace("kevent wait %d return %d errno %d used %lld millsecond", timeoutMs,
        numEvents, errno, easynet::now() - ticks);
  fatalif(numEvents == -1 && errno != EINTR, "kevent return error %d %s", errno,
          strerror(errno));

  if (numEvents > 0) {
    fillActiveChannels(numEvents, activeChannels);
    // if (implicit_cast<size_t>(numEvents) == events_.size()) {
    //   events_.resize(events_.size() * 2);
    // }
  } else if (numEvents == 0) {
    trace("nothing happended");
  } else {
    error("EPollPoller::poll()");
  }
}

void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) {
  assert(static_cast<size_t>(numEvents) <= m_events.size());

  unsigned int event_one = 0;
  for (int i = 0; i < numEvents; ++i) {
    Channel* channel = static_cast<Channel*>(m_events[i].udata);
    event_one = 0;
    if (m_events[i].filter & EVFILT_READ) {
      event_one |= POLLIN;
    }
    if (m_events[i].filter & EVFILT_WRITE) {
      event_one |= POLLOUT;
    }

    if (m_events[i].flags & EV_ERROR) {
      event_one |= POLLERR;
    }

    if (m_events[i].flags & EV_EOF) {
      event_one |= POLLIN;
    }
    channel->setRevents(event_one);
    activeChannels->push_back(channel);
  }
}

void EPollPoller::updateChannel(Channel* channel) {
  assertInLoopThread();
  trace("fd = %d events = %d", channel->fd(), channel->events());
  const int index = channel->index();
  if (index == kNew || index == kDeleted) {
    int fd = channel->fd();
    // index == kNew
    if (index == kNew) {
      assert(m_channels.find(fd) == m_channels.end());
      m_channels[fd] = channel;
    }
    // index == kDeleted
    else {
      assert(m_channels.find(fd) != m_channels.end());
      assert(m_channels[fd] == channel);
    }
    channel->setIndex(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    int fd = channel->fd();
    (void)fd;
    assert(m_channels.find(fd) != m_channels.end());
    assert(m_channels[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPollPoller::removeChannel(Channel* channel) {
  assertInLoopThread();
  int fd = channel->fd();
  trace("fd = %d", fd);
  assert(m_channels.find(fd) != m_channels.end());
  assert(m_channels[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = m_channels.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->setIndex(kNew);
}

void EPollPoller::update(int operation, Channel* channel) {
  struct timespec now;
  now.tv_nsec = 0;
  now.tv_sec = 0;

  struct kevent kev[2];
  bzero(&kev, sizeof kev);

  int fd = channel->fd();
  int n = 0;

  // EPOLL_CTL_ADD
  if (operation == EPOLL_CTL_ADD) {
    EV_SET(&kev[n++], fd, EVFILT_READ,
           EV_ADD | (channel->readEnabled() ? EV_ENABLE : EV_DISABLE), 0, 0,
           channel);
    EV_SET(&kev[n++], fd, EVFILT_WRITE,
           EV_ADD | (channel->writeEnabled() ? EV_ENABLE : EV_DISABLE), 0, 0,
           channel);
  }
  // EPOLL_CTL_DEL
  else if (operation == EPOLL_CTL_DEL) {
    EV_SET(&kev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, channel);
    EV_SET(&kev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, channel);
  }
  // EPOLL_CTL_MOD
  else if (operation == EPOLL_CTL_MOD) {
    EV_SET(&kev[n++], fd, EVFILT_READ,
           channel->readEnabled() ? EV_ENABLE : EV_DISABLE, 0, 0, channel);
    EV_SET(&kev[n++], fd, EVFILT_WRITE,
           channel->writeEnabled() ? EV_ENABLE : EV_DISABLE, 0, 0, channel);
  }

  trace("modifying channel %lld fd %d events read %d write %d epoll %d",
        (long long)channel->index(), channel->fd(), channel->readEnabled(),
        channel->writeEnabled(), m_epoll_fd);
  int r = kevent(m_epoll_fd, kev, n, NULL, 0, &now);
  fatalif(r, "kevent mod failed %d %s", errno, strerror(errno));
}

#elif defined(OS_LINUX)

EPollPoller::EPollPoller(EventLoop* loop)
    : m_loop(loop),
      m_epoll_fd(::epoll_create1(EPOLL_CLOEXEC)),
      m_events(kInitEventListSize) {
  if (m_epoll_fd < 0) {
    fatal("EPollPoller::EPollPoller");
  }
}

EPollPoller::~EPollPoller() { ::close(m_epoll_fd); }

void EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
  int numEvents = ::epoll_wait(m_epoll_fd, m_events.data(),
                               static_cast<int>(m_events.size()), timeoutMs);

  int64_t ticks = easynet::now();

  trace("epoll wait %d return %d errno %d used %lld millsecond", timeoutMs,
        numEvents, errno, static_cast<long long int>(easynet::now() - ticks));
  fatalif(numEvents == -1 && errno != EINTR, "kevent return error %d %s", errno,
          strerror(errno));

  if (numEvents > 0) {
    fillActiveChannels(numEvents, activeChannels);
    if (static_cast<size_t>(numEvents) == m_events.size()) {
      m_events.resize(m_events.size() * 2);
    }
  } else if (numEvents == 0) {
    trace("nothing happended");
  } else {
    error("EPollPoller::poll()");
  }
}

void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) {
  assert(static_cast<size_t>(numEvents) <= m_events.size());
  for (int i = 0; i < numEvents; ++i) {
    Channel* channel = static_cast<Channel*>(m_events[i].data.ptr);
#ifndef NDEBUG
    int fd = channel->fd();
    ChannelMap::const_iterator it = m_channels.find(fd);
    assert(it != m_channels.end());
    assert(it->second == channel);
#endif
    channel->setRevents(m_events[i].events);
    activeChannels->push_back(channel);
  }
}

void EPollPoller::updateChannel(Channel* channel) {
  assertInLoopThread();
  trace("fd = %d events = %d", channel->fd(), channel->events());
  const int index = channel->index();
  if (index == kNew || index == kDeleted) {
    int fd = channel->fd();
    // index == kNew
    if (index == kNew) {
      assert(m_channels.find(fd) == m_channels.end());
      m_channels[fd] = channel;
    }
    // index == kDeleted
    else {
      assert(m_channels.find(fd) != m_channels.end());
      assert(m_channels[fd] == channel);
    }
    channel->setIndex(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    int fd = channel->fd();
    (void)fd;
    assert(m_channels.find(fd) != m_channels.end());
    assert(m_channels[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPollPoller::removeChannel(Channel* channel) {
  assertInLoopThread();
  int fd = channel->fd();
  trace("fd = %d", fd);
  assert(m_channels.find(fd) != m_channels.end());
  assert(m_channels[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = m_channels.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->setIndex(kNew);
}

void EPollPoller::update(int operation, Channel* channel) {
  struct epoll_event event;
  bzero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  if (::epoll_ctl(m_epoll_fd, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      error("epoll_ctl op=%d fd=%d", operation, fd);
    } else {
      fatal("epoll_ctl op=%d fd=%d", operation, fd);
    }
  }
}
#endif
}  // namespace easynet