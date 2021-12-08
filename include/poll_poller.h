/*
 * @Author: lei
 * @Description: IO复用机制选择poll的实现Poller
 * @FilePath: /MyNetLib/include/poll_poller.h
 */
#ifndef POLL_POLLER_H
#define POLL_POLLER_H

#include <sys/poll.h>

#include <unordered_map>
#include <vector>

#include "channel.h"
#include "noncopyable.h"
#include "poller.h"
#include "time_stamp.h"

class PollPoller : public Poller {
 public:
  using PollList = std::vector<pollfd>;
  PollPoller(EventLoop *loop) : Poller(loop) {}
  ~PollPoller() = default;

  TimeStamp Polling(int timeout_ms, ChannelList *active_channels) override;
  void UpdateChannel(Channel *channel) override;
  void RemoveChannel(Channel *channel) override;

 private:
  void FillActiveChannnels(int num_events, ChannelList *active_channels);

  PollList poll_list_;
};

#endif  // POLL_POLLER_H