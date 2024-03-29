#pragma once
#include "functional"
#include "timestamp.h"
#include "util.h"
namespace easynet {
class EventLoop;

class Channel : private noncopyable {
 public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void(TimeStamp)> ReadEventCallback;

  // EventLoop为事件管理器，fd为通道内部的fd
  Channel(EventLoop *loop, int fd);
  ~Channel();

  void handleEvent(TimeStamp receiveTime);
  void setReadCallback(const ReadEventCallback &cb) { readCallback = cb; }
  void setWriteCallback(const EventCallback &cb) { writeCallback = cb; }
  void setErrorCallback(const EventCallback &cb) { errorCallback = cb; }
  void setCloseCallback(const EventCallback &cb) { closeCallback = cb; }

  int fd() const { return m_fd; }

  void setIndex(int index) { idx = index; }
  int index() const { return idx; }

  int events() const { return m_events; }

  void setRevents(int revents) { m_revents = revents; }

  bool isNoneEvent() const { return m_events == kNoneEvent; }
  bool readEnabled() const { return m_events & kReadEvent; }
  bool writeEnabled() const { return m_events & kWriteEvent; }

  void enableRead() {
    m_events |= kReadEvent;
    update();
  }

  void enableWrite() {
    m_events |= kWriteEvent;
    update();
  }

  void disableWrite() {
    m_events &= ~kWriteEvent;
    update();
  }

  void disableAll() {
    m_events = kNoneEvent;
    update();
  }

  bool isWriting() const { return m_events & kWriteEvent; }
  EventLoop *ownerLoop() const { return m_loop; }

 private:
  void update();

  static const int kReadEvent;
  static const int kWriteEvent;
  static const int kNoneEvent;

  EventLoop *m_loop;
  int idx;
  int m_fd;
  int m_events;   // 关心的IO事件
  int m_revents;  // 目前活动的事件，如struct pollfd 的revents
  ReadEventCallback readCallback;
  EventCallback writeCallback, errorCallback, closeCallback;
  bool m_eventHandling;
};
}  // namespace easynet