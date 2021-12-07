/*
 * @Author: lei
 * @Description: 定时器类型定义
 * @FilePath: /MyNetLib/include/timer.h
 */
#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <mutex>

using TimeEventCallback = std::function<void()>;
class Timer {
 public:
  Timer(unsigned long long expired_time_ms, int32_t repeated_times,
        int64_t interval, TimeEventCallback& callback);
  ~Timer() = default;

  void SetExpiredTime(unsigned long long expired_time) {
    expired_time_ = expired_time;
  }
  unsigned long long ExpiredTime() const { return expired_time_; }
  int64_t Id() const { return id_; }
  int64_t Interval() const { return interval_; }
  int32_t RepeatTimes() const { return repeated_times_; }
  void DecRepeatTimes() { repeated_times_--; }

  void Run();

 private:
  static int64_t GenerateId();

 private:
  int64_t id_;
  unsigned long long expired_time_;
  int32_t repeated_times_;  // 重复计时次数，如果只计时一次，那么该值为0
  int64_t interval_;
  TimeEventCallback callback_;

  static int64_t initial_id_;
  static std::mutex mutex_;  // 用于实现initial_id_的互斥访问
};

#endif  // TIMER_H