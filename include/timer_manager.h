/*
 * @Author: lei
 * @Description: 定时器容器类型定义，使用小顶堆(std::priority_queue)实现，
 * 管理一个或多个定时器Timer对象;其他外部程序,
 * 只使用该类提供的接口，感知不到Timer类;
 * @FilePath: /MyNetLib/include/timer_manager.h
 */

#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <queue>
#include <vector>

#include "timer.h"

// 一个EventLoop拥有一个TimerManager对象,通过TimerManager类操作其管理的定时器
class TimerManager {
 public:
  TimerManager() = default;
  ~TimerManager() = default;
  Timer* AddTimer(int32_t repeated_times, int64_t interval,
                  TimeEventCallback& callback);
  void RemoveTimer(Timer* timer);
  void CheckAndHandle();
  // 获取最近的超时时间，用于设置epoll等io复用调用中的超时时间
  unsigned long long GetRecentTimeout();

 private:
  unsigned long long GetCurrentMillisecs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000);
  }

 private:
  // 作为timer_que_的排序算子，过期时间更早、重复计时次数更少的Timer对象排前面
  struct cmp {
    bool operator()(Timer* t1, Timer* t2) const {
      return t1->ExpiredTime() == t2->ExpiredTime()
                 ? t1->RepeatTimes() > t2->RepeatTimes()
                 : t1->ExpiredTime() > t2->ExpiredTime();
    }
  };
  std::priority_queue<Timer*, std::vector<Timer*>, cmp> timers_que_;
};

#endif  // TIMER_MANAGER_H