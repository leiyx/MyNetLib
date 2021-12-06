/*
 * @Author: lei
 * @Description: 时间戳类
 * @FilePath: /MyNetLib/include/time_stamp.h
 */

#ifndef TIME_STAMP_H
#define TIME_STAMP_H

#include <string>

class TimeStamp {
 public:
  TimeStamp();
  explicit TimeStamp(int64_t micro_seconds_since_epoch);
  std::string TimeToString() const;

  static TimeStamp Now();

 private:
  int64_t micro_seconds_since_epoch_;
};

#endif  // TIME_STAMP_H
// TODO:对时间戳类的优化：临时对象的构造与析构、每次调用系统调用time