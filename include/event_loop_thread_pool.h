/*
 * @Author: lei
 * @Description: 多个EventLoopThread组成线程池
 * @FilePath: /MyNetLib/include/event_loop_thread_pool.h
 */
#ifndef EVENT_LOOP_THREAD_POOL_H
#define EVENT_LOOP_THREAD_POOL_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "callbacks.h"
#include "noncopyable.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
 public:
  EventLoopThreadPool(EventLoop *base_loop, const std::string &name_arg);
  ~EventLoopThreadPool();

  void SetThreadNum(int num_thread) { num_threads_ = num_thread; }

  void Start(const ThreadInitCallback &ticb = ThreadInitCallback());

  // 如果工作在多线程中，baseLoop_默认以轮询的方式分配channel给subloop
  EventLoop *GetNextLoop();

  std::vector<EventLoop *> GetAllLoops();

  bool Started() const { return started_; }
  const std::string Name() const { return name_; }

 private:
  EventLoop *base_loop_;
  std::string name_;  // base_loop_对应的thread的名称

  bool started_;
  int num_threads_;
  int next_;

  // one loop per thread
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *> loops_;
};

#endif  // EVENT_LOOP_THREAD_POOL_H