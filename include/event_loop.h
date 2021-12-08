/*
 * @Author: lei
 * @Description: 事件循环，作为Channel和Poller的桥梁
 * @FilePath: /MyNetLib/include/event_loop.h
 */

#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "callbacks.h"
#include "current_thread.h"
#include "time_stamp.h"
#include "timer_manager.h"

class Channel;
class Poller;

// one loop one thread
class EventLoop {
 public:
  using Functor = std::function<void()>;
  using TimeEventCallback = std::function<void()>;

  EventLoop();
  ~EventLoop();

  void Loop();
  void Quit();

  TimeStamp RollReturnTime() const { return poll_return_time_; }

  void RunInLoop(Functor cb);  // 在当前loop中执行cb
  void QueueInLoop(Functor cb);  // 把cb放入队列中，并唤醒loop所在的线程

  void Wakeup();  // 唤醒一个线程，即唤醒一个Eventloop（一个subReactor）
  bool IsInLoopThread() const { return thread_id_ == CurrentThread::Tid(); }

  // 调用Poller相关接口
  void UpdateChannel(Channel* channel);
  void RemoveChannel(Channel* channel);
  void HasChannel(Channel* channel);

  // 定时任务相关接口
  // TODO：是否可以返回id替代返回Timer*
  Timer* RunAfter(int32_t repeated_times, int64_t interval,
                  TimeEventCallback callback);

 private:
  void HandleRead();  // wakeup_channel_的读事件处理回调
  void DoPendingFunctors();
  void CheckAndHandleTimerEvent();
  void PollAndHandleIoEvent();

 private:
  using ChannelList = std::vector<Channel*>;

  std::atomic_bool looping_;
  std::atomic_bool quit_;
  const int32_t thread_id_;

  TimeStamp poll_return_time_;
  std::unique_ptr<Poller> poller_;

  // 该EventLoop管理的所有Channel，Poller返回的有事件的channel！
  ChannelList active_channels_;

  // 当MainLoop获取一个新连接时要分发给睡眠中的SubLoop，SubLoop需要被唤醒
  int wake_fd_;                              // 用于唤醒阻塞的EventLoop
  std::unique_ptr<Channel> wakeup_channel_;  // 包装wake_fd_的channel

  std::vector<Functor> pending_functors_;  // 回调队列，非线程安全
  std::mutex mutex_;                       // 用于保护pending_functors_

  std::atomic_bool calling_pending_functors_;  // 标记当前loop是否有回调需要执行

  TimerManager timer_manager_;
};

#endif