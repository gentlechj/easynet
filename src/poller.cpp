#include "poller.h"

#include "Channel.h"
#include "logging.h"

namespace easynet {
Poller::Poller(EventLoop* loop) : m_loop(loop) {}
Poller::~Poller() {}

void Poller::poll(int timeoutMs, ChannelList* activeChannels) {
  int numEvents = ::poll(&*m_pollfds.begin(), m_pollfds.size(), timeoutMs);

  if (numEvents > 0) {
    trace("%d events happened", numEvents);
    fillActiveChannels(numEvents, activeChannels);
  } else if (numEvents == 0) {
    trace("No events happened");
  } else {
    error("POLLer::poll() failed");
  }
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) {
  for (auto pfd = m_pollfds.cbegin(); pfd != m_pollfds.end() && numEvents > 0;
       pfd++) {
    if (pfd->revents > 0) {
      --numEvents;
      auto ch = m_channels.find(pfd->fd);
      assert(ch != m_channels.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->setRevents(pfd->revents);
      activeChannels->push_back(channel);
    }
  }
}

void Poller::updateChannel(Channel* channel) {
  assertInLoopThread();
  trace("fd = %d events = %d", channel->fd(), channel->events());
  if (channel->index() < 0) {
    // 新channel，添加到poller
    assert(m_channels.find(channel->fd()) == m_channels.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    m_pollfds.push_back(pfd);
    int idx = static_cast<int>(m_pollfds.size()) - 1;
    m_channels[channel->fd()] = channel;
  } else {
    // 更新已经存在的channel
    assert(m_channels.find(channel->fd()) != m_channels.end());
    assert(m_channels[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(m_pollfds.size()));
    struct pollfd& pfd = m_pollfds[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -1);
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->isNoneEvent()) {
      //  如果某一个channel暂时不关心任何事件，那么可以吧pollfd.fd设置为负数，这样poll会忽略此文件描述符
      //  不能改为把pollfd.events设为0，否则无法屏蔽POLLERR事件
      //  没有关注的事件时候，忽略此fd
      //  TODO 改进设为channel->fd()的相反数减一，检查不变量（文件描述符从0开始，减一是为了兼容0，因为0的相反数还是0）
      pfd.fd = -1;
    }
  }
}

}  // namespace easynet