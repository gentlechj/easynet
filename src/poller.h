#pragma once
#include <vector>

#include "util.h"
namespace easynet {
class Channel;

// 封住IO多路复用 with poll
class Poller : private noncopyable {
 public:
  typedef std::vector<Channel*> ChannelList;
  Poller() = default;
  virtual ~Poller() = default;

  // Polls IO 事件
  virtual void poll(int timeoutMs, ChannelList* activeChannels) = 0;

  // 改变Channel感兴趣的IO事件
  virtual void updateChannel(Channel* channel) = 0;

  // 移除channel
  virtual void removeChannel(Channel* channel) = 0;
};

}  // namespace easynet