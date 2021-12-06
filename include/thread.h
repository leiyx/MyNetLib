/*
 * @Author: lei
 * @Description: 对C++11的thread作一层包装
 * @FilePath: /MyNetLib/include/thread.h
 */
#ifndef THREAD_H
#define THREAD_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "noncopyable.h"

class Thread : noncopyable {
 public:
  using ThreadFunc = std::function<void()>;
  Thread(ThreadFunc func, const std::string &name = std::string());
  ~Thread();

  // 与C++11thread不同，该THread类对象只有Start方法被调用才会开始运行其绑定函数
  void Start();
  void Join();

  std::string GetName() const { return name_; }
  pid_t GetTid() const { return tid_; }
  bool GetStarted() const { return started_; }
  static int GetNumCreated() { return num_created_; }
  void SetDefaultName();

 private:
  pid_t tid_;
  std::string name_ = "";

  bool joined_;
  bool started_;

  ThreadFunc func_;

  std::unique_ptr<std::thread> thread_;
  // C++11中std::thread一被构造，线程就开始运行，控制不了它的启动时机,因此这里使用智能指针
  static std::atomic_int num_created_;
  // 作为类的一个静态数据成员，记录产生的线程数,属于多个线程的共享资源，使用atomic_int保证线程安全
};

#endif  // THREAD_H