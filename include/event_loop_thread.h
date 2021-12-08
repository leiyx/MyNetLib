/*
 * @Author: lei
 * @Description: 在Thread类上的一层封装，one loop per
 * thread,将一个EventLoop与一个Thread对应起来
 * @FilePath: /MyNetLib/include/event_loop_thread.h
 */
#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

#include <condition_variable>
#include <functional>
#include <mutex>

#include "noncopyable.h"
#include "thread.h"

class EventLoop;
class EventLoopThread : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = std::string());
  ~EventLoopThread();
  EventLoop* StartLoop();

 private:
  void ThreadFunc();  // 线程运行函数

  EventLoop* loop_;
  Thread thread_;
  bool exiting_;

  std::mutex mutex_;
  std::condition_variable cond_;

  ThreadInitCallback thread_init_callback_;
};

#endif  // EVENT_LOOP_THREAD_H