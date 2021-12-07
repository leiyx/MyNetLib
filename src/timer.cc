#include "timer.h"

int64_t Timer::initial_id_ = 0;
std::mutex Timer::mutex_{};

Timer::Timer(unsigned long long expired_time_ms, int32_t repeated_times,
             int64_t interval, TimeEventCallback& callback)
    : expired_time_(expired_time_ms),
      repeated_times_(repeated_times),
      interval_(interval),
      callback_(std::move(callback)) {
  id_ = GenerateId();
}

void Timer::Run() { callback_(); }

int64_t Timer::GenerateId() {
  {
    std::lock_guard<std::mutex> lk(mutex_);
    initial_id_++;
  }
  return initial_id_;
}