#pragma once
#include <map>
#include <vector>

#include "eventloop.h"
#include "poller.h"

#ifdef OS_LINUX
struct epoll_event;
#elif defined(OS_MACOSX)
struct kevent;
#else
#error "platform unsupported"
#endif

namespace easynet {
class Channel;

class EPollPoller : public Poller {
 public:
  EPollPoller(EventLoop* loop);
  ~EPollPoller();

  // Polls IO 事件
  virtual void poll(int timeoutMs, ChannelList* activeChannels) override;

  // 改变Channel感兴趣的IO事件
  virtual void updateChannel(Channel* channel) override;

  // 移除channel
  virtual void removeChannel(Channel* channel) override;

  void assertInLoopThread() { m_loop->assertInLoopThread(); };

 private:
  static const int kInitEventListSize = 16;

  // 找出活动的channel，放入activeChannels
  void fillActiveChannels(int numEvents, ChannelList* activeChannels);

  void update(int operation, Channel* channel);

#ifdef OS_LINUX
  typedef std::vector<struct epoll_event> EventList;
#elif defined(OS_MACOSX)
  typedef std::vector<struct kevent> EventList;
#endif

  typedef std::map<int, Channel*> ChannelMap;

  EventLoop* m_loop;
  int m_epoll_fd;
  EventList m_events;
  ChannelMap m_channels;  // fd到Channel* 的映射
};
}  // namespace easynet