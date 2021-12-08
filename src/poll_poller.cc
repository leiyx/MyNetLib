#include "poll_poller.h"

#include <strings.h>
#include <sys/poll.h>

#include <algorithm>

#include "logger.h"

// Channel类中的成员index，表示该Channel在poll_list_的下标;

TimeStamp PollPoller::Polling(int timeout_ms, ChannelList* active_channels) {
  int nready = ::poll(&*poll_list_.begin(), poll_list_.size(), timeout_ms);
  int saved_errno = errno;
  TimeStamp now(TimeStamp::Now());
  if (nready > 0) {
    FillActiveChannnels(nready, active_channels);
  } else if (nready == 0) {
    LOG_DEBUG << "no events occured! in " << timeout_ms << " ms";
  } else {
    if (saved_errno == EINTR) {
      errno = saved_errno;
      LOG_ERROR << "PollPoller::poll() EINTR";
    }
    LOG_FATAL << "other unkonwn error occured!";
  }
  return now;
}

void PollPoller::FillActiveChannnels(int nready, ChannelList* active_channels) {
  Channel* channel;
  for (PollList::const_iterator poll_fd_iter = poll_list_.cbegin();
       poll_fd_iter != poll_list_.end() && nready > 0; ++poll_fd_iter) {
    if (poll_fd_iter->revents > 0) {
      ChannelMap::const_iterator ch = channel_maps_.find(poll_fd_iter->fd);
      if (ch != channel_maps_.end()) {
        channel = ch->second;
        channel->Set_Revents(poll_fd_iter->revents);
        active_channels->emplace_back(channel);
        nready--;
      }
    }
  }
}

void PollPoller::UpdateChannel(Channel* channel) {
  if (channel->Index() < 0) {
    // Channel第一次被加入监听
    pollfd poll_fd;
    poll_fd.fd = channel->Fd();
    poll_fd.events = channel->Events();
    poll_fd.revents = 0;
    poll_list_.push_back(poll_fd);

    channel->Set_Index(static_cast<int>(poll_list_.size() - 1));

    channel_maps_[channel->Fd()] = channel;
  } else {
    // Channel已被监听，只是更新监听事件events
    int idx = channel->Index();
    pollfd& poll_fd = poll_list_[idx];
    poll_fd.fd = channel->Fd();
    poll_fd.events = channel->Events();
    poll_fd.revents = 0;
  }
}

void PollPoller::RemoveChannel(Channel* channel) {
  int idx = channel->Index();
  if (idx == poll_list_.size() - 1) {
    poll_list_.pop_back();
  } else {
    int channel_fd_at_end = poll_list_.back().fd;
    std::iter_swap(poll_list_.begin() + idx, poll_list_.end() - 1);
    channel_maps_[channel_fd_at_end]->Set_Index(idx);
    poll_list_.pop_back();
  }
}