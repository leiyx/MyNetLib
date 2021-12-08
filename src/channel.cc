#include "channel.h"

#include <sys/epoll.h>
#include <sys/poll.h>

#include "event_loop.h"
#include "logger.h"
#ifdef USE_POLL
int Channel::kReadEvent = POLLIN | POLLPRI;
int Channel::kWriteEvent = POLLOUT;
int Channel::kNoneEvent = 0;
#elif defined USE_EPOLL  // USE_EPOLL
int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
int Channel::kWriteEvent = EPOLLOUT;
int Channel::kNoneEvent = 0;
#endif

static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {}

void Channel::HandleEvent(TimeStamp received_time) {
  if (tied_) {
    auto temp = tie_.lock();
    // 如果TcpConnection被手动Remove时,即temp为空，就不再处理fd的事件
    if (temp) HandleEventWithGuard(received_time);
  } else
    HandleEventWithGuard(received_time);
}

// 一个TcpConnection新连接创建的时候,TcpConnection::ConnectionEstablished调用Channel::Tie
void Channel::Tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::HandleEventWithGuard(TimeStamp received_time) {
  LOG_DEBUG << "Channel::HandleEventWithGuard : " << revents_;
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOG_DEBUG << "Channel::HandleEventWithGuard  -- > 连接关闭";
    if (connection_close_callback_) {
      connection_close_callback_();
    }
  }

  if (revents_ & POLLERR) {
    LOG_DEBUG << "Channel::HandleEventWithGuard  -- > 发生错误";
    if (error_callback_) {
      error_callback_();
    }
  }

  if (revents_ & (POLLIN | POLLPRI)) {
    LOG_DEBUG << "Channel::HandleEventWithGuard  -- > 数据可读";
    if (read_event_callback_) {
      read_event_callback_(received_time);
    }
  }

  if (revents_ & POLLOUT) {
    LOG_INFO << "Channel::HandleEventWithGuard  -- > 数据可写";
    if (write_event_callback_) {
      write_event_callback_();
    }
  }
}

void Channel::Update() { loop_->UpdateChannel(this); }
void Channel::Remove() { loop_->RemoveChannel(this); }