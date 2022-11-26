#pragma once
#include <sys/time.h>

#include <map>
#include <vector>

#include "eventloop.h"
#include "util.h"

struct pollfd;

namespace easynet {
class Channel;

// 封住IO多路复用 with poll
class Poller : private noncopyable {
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  ~Poller();

  // Polls IO 事件
  void poll(int timeoutMs, ChannelList* activeChannels);

  // 改变Channel感兴趣的IO事件
  void updateChannel(Channel* channel);
  
  // 移除channel
  void removeChannel(Channel* channel);

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