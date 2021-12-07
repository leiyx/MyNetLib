#include "timer_manager.h"

Timer* TimerManager::AddTimer(int32_t repeated_times, int64_t interval,
                              TimeEventCallback& callback) {
  if (interval <= 0) return nullptr;
  unsigned long long now = GetCurrentMillisecs();
  Timer* t = new Timer(now + interval, repeated_times, interval, callback);
  timers_que_.push(t);
  return t;
}

void TimerManager::RemoveTimer(Timer* timer) {
  std::priority_queue<Timer*, std::vector<Timer*>, cmp> newqueue;

  while (!timers_que_.empty()) {
    Timer* top = timers_que_.top();
    timers_que_.pop();
    if (top == timer) {
      delete timer;
      continue;
    }
    newqueue.push(top);
  }
  timers_que_ = std::move(newqueue);
}

void TimerManager::CheckAndHandle() {
  unsigned long long now = GetCurrentMillisecs();
  Timer* timer;
  while (!timers_que_.empty()) {
    timer = timers_que_.top();
    if (timer->ExpiredTime() <= now) {
      timer->Run();
      if (timer->RepeatTimes() == 0) {
        timers_que_.pop();
        delete timer;
      } else {
        timer->SetExpiredTime(now + timer->Interval());
        timer->DecRepeatTimes();
        timers_que_.pop();
        timers_que_.push(timer);
      }
    } else  // 后面的定时器也一定没有过期，因为timer_queue_中的定时器是按过期时间由低到高排序
      return;
  }
}

unsigned long long TimerManager::GetRecentTimeout() {
  unsigned long long timeout = -1;
  if (timers_que_.empty()) return timeout;

  unsigned long long now = GetCurrentMillisecs();
  timeout = timers_que_.top()->ExpiredTime() - now;
  if (timeout < 0) timeout = 0;

  return timeout;
}