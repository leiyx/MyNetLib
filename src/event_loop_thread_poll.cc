/*
 * @Author: your name
 * @Description:
 * @FilePath: /MyNetLib/src/event_loop_thread_poll.cc
 */
#include <memory>

#include "event_loop_thread.h"
#include "event_loop_thread_pool.h"
#include "logger.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop,
                                         const std::string &name_arg)
    : base_loop_(base_loop),
      name_(name_arg),
      started_(false),
      num_threads_(0),
      next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

// EventLoopThreadPool::Start --> EventLoopThread::StartLoop -->Thread::Start
void EventLoopThreadPool::Start(const ThreadInitCallback &cb) {
  started_ = true;
  for (int i = 0; i < num_threads_; ++i) {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
    EventLoopThread *t = new EventLoopThread(cb, buf);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(
        t->StartLoop());  // 底层创建线程，绑定一个新的EventLoop，并返回该loop的地址
  }
  LOG_INFO << "ThreadPool started!";
  // 整个服务端只有一个线程，运行着baseloop
  if (num_threads_ == 1 && cb) {
    cb(base_loop_);
  }
}

// 如果工作在多线程中，base_loop_默认以轮询的方式分配channel给subloop,负载均衡算法：轮询
EventLoop *EventLoopThreadPool::GetNextLoop() {
  EventLoop *loop = base_loop_;
  if (!loops_.empty())  // 通过轮询获取下一个处理事件的loop
  {
    loop = loops_[next_];
    ++next_;
    if (next_ >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::GetAllLoops() {
  if (loops_.empty()) {
    return std::vector<EventLoop *>(1, base_loop_);
  } else {
    return loops_;
  }
}