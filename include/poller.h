/*
 * @Author: lei
 * @Description: Reactor模型中的事件分发器
 * @FilePath: /MyNetLib/include/poller.h
 */
#ifndef POLLER_H
#define POLLER_H

#include <unordered_map>
#include <vector>

#include "noncopyable.h"

class EventLoop;
class Channel;
class TimeStamp;

// Poller是一个纯虚基类，是对IO复用的抽象,只负责事件分发
class Poller : noncopyable {
 public:
  // 对于ChannelMap，键为socketfd,值为socketfd对应的Channel的指针
  using ChannelMap = std::unordered_map<int, Channel*>;
  using ChannelList = std::vector<Channel*>;

  Poller(EventLoop* loop) : loop_(loop) {}
  virtual ~Poller() = default;

  // 一组公共IO复用接口，必须由子类重写，不同的子类（IO复用机制）有不同实现
  virtual TimeStamp Polling(int timeout_ms, ChannelList* active_channels) = 0;
  virtual void UpdateChannel(Channel* channel) = 0;
  virtual void RemoveChannel(Channel* channel) = 0;

  bool HasChannel(Channel* channel) const;

  // 在default_poller.cc中实现
  static Poller* NewDefaultPoller(EventLoop* loop);

 protected:
  ChannelMap channel_maps_;

 private:
  EventLoop* loop_;
};

#endif  // POLLER_H