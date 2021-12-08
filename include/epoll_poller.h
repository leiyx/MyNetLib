/*
 * @Author: lei
 * @Description: IO复用机制选择epoll的实现Poller
 * @FilePath: /MyNetLib/include/epoll_poller.h
 */
#ifndef EPOLL_POLLER_H
#define EPOLL_POLLER_H

#include <sys/epoll.h>

#include <vector>

#include "poller.h"
#include "time_stamp.h"

class EpollPoller : public Poller {
 public:
  using EventList = std::vector<epoll_event>;
  EpollPoller(EventLoop *loop);
  ~EpollPoller() override;

  TimeStamp Polling(int timeout_ms, ChannelList *active_channels) override;
  void UpdateChannel(Channel *channels) override;
  void RemoveChannel(Channel *channels) override;

 private:
  static const int kInitEventSize = 16;

  void FillActiveChannels(int num_events, ChannelList *active_channels) const;
  void Update(int operation, Channel *channel);

  int epoll_fd_;
  EventList events_;  // 用于存放epoll_wait返回的struct epooll_events数组
};

#endif  // EPOLL_POLLER_H