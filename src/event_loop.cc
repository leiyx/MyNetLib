#include "event_loop.h"

#include <errno.h>
#include <unistd.h>

#include <memory>

#include "channel.h"
#include "logger.h"
#include "poller.h"
#include "utils.h"

// 避免一个线程创建多个EventLoop,起到thread_local的作用
__thread EventLoop* t_loopInThisThread = nullptr;

// 定义默认的Poller的IO复用接口的超时时间
const int kPollTimeMs = 5000;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      wake_fd_(utils::CreateEventfd()),
      wakeup_channel_(new Channel(this, wake_fd_)),
      poller_(Poller::NewDefaultPoller(this)),
      thread_id_(CurrentThread::Tid()),
      calling_pending_functors_(false) {
  LOG_DEBUG << "EventLoop created " << this << " in thread " << thread_id_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << thread_id_;
  } else {
    t_loopInThisThread = this;
  }
  wakeup_channel_->SetReadEventCallback(
      std::bind(&EventLoop::HandleRead, this));
  wakeup_channel_->EnableRead();
}

EventLoop::~EventLoop() {
  wakeup_channel_->DisableAll();
  wakeup_channel_->Remove();
  ::close(wake_fd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::Loop() {
  looping_ = true;
  quit_ = false;
  while (!quit_) {
    CheckAndHandleTimerEvent();  // 处理定时事件
    PollAndHandleIoEvent();      // 处理IO事件
    DoPendingFunctors();         // 处理其他事件
  }
  looping_ = false;
}

void EventLoop::Quit() {
  quit_ = true;
  LOG_DEBUG << "EventLoop in thread " << thread_id_ << " has quit";
  if (!IsInLoopThread()) {
    Wakeup();
  }
}

void EventLoop::CheckAndHandleTimerEvent() { timer_manager_.CheckAndHandle(); }

void EventLoop::PollAndHandleIoEvent() {
  active_channels_.clear();
  // 如果有定时任务，则将IO复用超时时间设置的当前最小定时时间
  int timeout_ms = timer_manager_.GetRecentTimeout() == -1
                       ? kPollTimeMs
                       : timer_manager_.GetRecentTimeout();
  poll_return_time_ = poller_->Polling(timeout_ms, &active_channels_);
  for (Channel* channel : active_channels_)
    channel->HandleEvent(poll_return_time_);
}

// 执行该EventLoop回调队列中所有待执行的回调函数
void EventLoop::DoPendingFunctors() {
  std::vector<Functor> functors;
  calling_pending_functors_ = true;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    functors.swap(pending_functors_);
  }
  for (const Functor& functor : functors) functor();
  calling_pending_functors_ = false;
}

void EventLoop::RunInLoop(Functor cb) {
  if (IsInLoopThread()) {  // 在当前loop对应线程中调用RunInLoop，直接执行回调cb即可
    cb();
  } else  // 在非当前loop线程中执行cb , 就需要唤醒loop所在线程，执行cb
  {
    QueueInLoop(cb);
  }
}

// 先把cb放入队列中，再唤醒loop所在的线程
void EventLoop::QueueInLoop(Functor cb) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    pending_functors_.emplace_back(cb);
  }
  // 当前loop正在执行回调，但是loop又有了新的回调,这时候就不能阻塞在io复用那，要再去执行一下doPendingFunctors函数
  if (!IsInLoopThread() || calling_pending_functors_) {
    Wakeup();
  }
}

void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wake_fd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}
void EventLoop::HandleRead() {
  uint64_t one = 1;
  ssize_t n = read(wake_fd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::wakeup() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::HasChannel(Channel* channel) { poller_->HasChannel(channel); }
void EventLoop::UpdateChannel(Channel* channel) {
  poller_->UpdateChannel(channel);
}
void EventLoop::RemoveChannel(Channel* channel) {
  poller_->RemoveChannel(channel);
}

Timer* EventLoop::RunAfter(int32_t repeated_times, int64_t interval,
                           TimeEventCallback callback) {
  return timer_manager_.AddTimer(repeated_times, interval, callback);
}
// TODO: 线程安全问题
