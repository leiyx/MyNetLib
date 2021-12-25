/*
 * @Author: lei
 * @Description: 基于Thread 实现one loop per thread
 * @FilePath: /MyNetLib/src/event_loop_thread.cc
 */
#include "event_loop_thread.h"

#include "event_loop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
                                 const std::string &name)
    : loop_(nullptr),
      exiting_(false),
      mutex_(),
      cond_(),
      thread_init_callback_(cb),
      thread_(std::bind(&EventLoopThread::ThreadFunc, this), name) {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != nullptr) {
    loop_->Quit();
    thread_.Join();
  }
}

EventLoop *EventLoopThread::StartLoop() {
  thread_.Start();
  EventLoop *loop = nullptr;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (loop_ == nullptr) cond_.wait(lock);
    loop = loop_;
  }
  return loop;
}

// one loop per thread 的具体实现
void EventLoopThread::ThreadFunc() {
  EventLoop loop;
  if (thread_init_callback_) thread_init_callback_(&loop);
  {
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = &loop;
    cond_.notify_one();
  }
  loop.Loop();
  std::unique_lock<std::mutex> lock(mutex_);
  loop_ = nullptr;
}