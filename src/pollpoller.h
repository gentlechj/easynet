#pragma once
#include <map>
#include <vector>

#include "eventloop.h"
#include "poller.h"
#include "util.h"

struct pollfd;

namespace easynet {
class Channel;

// 封住IO多路复用 with poll
class PollPoller : public Poller {
 public:
  typedef std::vector<Channel*> ChannelList;

  PollPoller(EventLoop* loop);
  ~PollPoller();

  // Polls IO 事件
  void poll(int timeoutMs, ChannelList* activeChannels) override;

  // 改变Channel感兴趣的IO事件
  void updateChannel(Channel* channel) override;

  // 移除channel
  void removeChannel(Channel* channel) override;

  void assertInLoopThread() { m_loop->assertInLoopThread(); };

 private:
  // 找出活动的channel，放入activeChannels
  void fillActiveChannels(int numEvents, ChannelList* activeChannels);

  typedef std::vector<struct pollfd> PollfdList;
  typedef std::map<int, Channel*> ChannelMap;

  EventLoop* m_loop;
  PollfdList m_pollfds;   // 缓存poll的fd
  ChannelMap m_channels;  // fd到Channel* 的映射
};
}  // namespace easynet