#include "channel.h"

#include "eventloop.h"
#include "logging.h"

namespace easynet {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : m_loop(loop), m_fd(fd), m_events(0), m_revents(0), idx(-1) {}

// void Channel::update() { m_loop->updateChannel(this); }

void Channel::handleEvent() {
  if (m_revents & POLLNVAL) {  // 文件描述符没有打开
    warn("Channel::handleEvent() POLLNVAL");
  }
  if (m_revents & (POLLERR | POLLNVAL)) {
    if (errorCallback) errorCallback();
  }
  if (m_revents & (POLLIN | POLLPRI)) {  // 高优先级可读
    if (readCallback) readCallback();
  }
  if (m_revents & (POLLOUT)) {
    if (writeCallback) writeCallback();
  }
}

void Channel::update() { m_loop->updateChannel(this); }
}  // namespace easynet