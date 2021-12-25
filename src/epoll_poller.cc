#include "epoll_poller.h"

#include <errno.h>
#include <strings.h>
#include <unistd.h>

#include "channel.h"
#include "logger.h"

// Channel类中的成员index意义：
// -1 表示该channel未添加到poller的channel_maps_中
const int kNew = -1;
// 1 表示该channel已添加到poller的channel_maps_中,并注册到epoll监听列表上
const int kAdded = 1;
// 2 表示该channel从epoll监听列表上移除，但依旧在poller的channel_maps_中
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop),
      epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventSize) {
  if (epoll_fd_ < 0) LOG_FATAL << "EPOLL_CREATE error: " << errno;
}

EpollPoller::~EpollPoller() { ::close(epoll_fd_); }

TimeStamp EpollPoller::Polling(int timeout_ms, ChannelList *active_channels) {
  LOG_DEBUG << "fd total count " << (int)channel_maps_.size();
  int ready_num = ::epoll_wait(epoll_fd_, &*events_.begin(),
                               static_cast<int>(events_.size()), timeout_ms);
  int saveErrno = errno;
  TimeStamp now(TimeStamp::Now());
  if (ready_num > 0) {
    LOG_DEBUG << ready_num << "events happened";
    FillActiveChannels(ready_num, active_channels);
    if (ready_num == events_.size()) events_.resize(events_.size() * 2);
  } else if (ready_num == 0) {
    LOG_DEBUG << "no events occured! in " << timeout_ms << "ms";
  } else {
    if (errno != EINTR) {
      errno = saveErrno;
      LOG_ERROR << "EPollPoller::poll() error";
    }
  }
  return now;
}

void EpollPoller::UpdateChannel(Channel *channel) {
  const int index = channel->Index();
  LOG_DEBUG << "fd = " << channel->Fd() << " events = " << channel->Events()
            << " index = " << index;
  if (index == kNew || index == kDeleted) {
    if (index == kNew) {
      int fd = channel->Fd();
      channel_maps_[fd] = channel;
    }
    channel->Set_Index(kAdded);
    Update(EPOLL_CTL_ADD, channel);
  } else  // channel在channel_maps_上注册过了
  {
    int fd = channel->Fd();
    if (channel->IsNoneEvent()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->Set_Index(kDeleted);
    } else
      Update(EPOLL_CTL_MOD, channel);
  }
}

// 1. 将channel从channel_maps_中移除
// 2. 置index为kNew
void EpollPoller::RemoveChannel(Channel *channel) {
  int fd = channel->Fd();
  channel_maps_.erase(fd);
  int index = channel->Index();
  if (index == kAdded) Update(EPOLL_CTL_DEL, channel);
  channel->Set_Index(kNew);
}

/**
 * @description:
 * 将活跃的连接，返回给EventLoop，在EventLoop里再对每个活跃的Channel调用hanndlEvent函数
 * @param {int} num_events:发生事件的channel数
 * @param {ChannelList}
 * *active_channels：内核返回的有事件的channel数组,由EventLoop类传入
 * @return {*}
 */
void EpollPoller::FillActiveChannels(int num_events,
                                     ChannelList *active_channels) const {
  for (int i = 0; i < num_events; i++) {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);  //****
    channel->Set_Revents(events_[i].events);
    active_channels->push_back(channel);
  }
}
/**
 * @description: Channel::update --> EventLoop::updateChannel -->
 * Poller::updateChannel --> Poller::update
 * @param {int} operation:EPOLL_ADD\EPOLL_DEL\EPOLL_MOD的一种
 *         到底是那种操作，updateChannel函数就是做这个事情，决定好operation后调用update
 * @param {Channel} *channel：要更新监听事件的那个Channel
 * @return {*}
 */
void EpollPoller::Update(int operation, Channel *channel) {
  epoll_event event;
  bzero(&event, sizeof event);
  int fd = channel->Fd();
  event.events = channel->Events();
  event.data.fd = fd;
  event.data.ptr = channel;

  if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_ERROR << "epoll_ctl del error: " << errno;
    } else {
      LOG_FATAL << "epoll_ctl add/mod error: " << errno;
    }
  }
}