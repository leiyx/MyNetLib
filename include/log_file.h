/*
 * @Author: lei
 * @Description: 对输出日志到文件的封装
 * @FilePath: /MyNetLib/include/log_file.h
 */
#ifndef LOG_FILE_H
#define LOG_FILE_H

#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <mutex>

// AppendFile类是对文件指针的一层包装，提供两个基本接口：Append,从buffer写数据到文件
// 和 Flush,冲刷文件到磁盘。该类使用了fwrite_unlock,非线程安全;
class AppendFile {
 public:
  explicit AppendFile(std::string& filename);
  ~AppendFile();

  void Append(const char* log_line, size_t len);
  void Flush();
  long WrittenBytes() const { return written_bytes_; }

 private:
  size_t Write(const char* log_line, size_t len);

  FILE* fp_;
  char buffer_[64 * 1024];
  long written_bytes_;  //在buffer_里写了多少字节
};

// LogFile类:将日志写到本地文件上;LogFile是对AppendFile的一层包装，在AppendFile的基础功能上
// 实现了 日志文件的分割 和 满足一定条件时冲刷文件到磁盘;
class LogFile {
 public:
  LogFile(const std::string& base_name, long roll_size, bool thread_safe = true,
          int flush_interval = 3, int check_every_n = 1024);
  ~LogFile();

  void Append(const char* logline, int len);
  void Flush();
  bool RollFile();

 private:
  void AppendUnlocked(const char* logline, int len);
  static std::string GetLogFileName(const std::string& filename, time_t* now);

 private:
  const std::string base_name_;
  const long roll_size_;
  const int flush_interval_;

  const int check_every_n_;
  int count_;

  time_t start_of_period_;  //开始记录日志的时间 ,某一天的起点
  time_t last_roll_;        //上一次滚动日志文件的时间
  time_t last_flush_;       //上一次日志写入文件的时间

  std::mutex mutex_;
  bool thread_safe_;

  const static int kRollPerSeconds_ = 60 * 60 * 24;

  std::unique_ptr<AppendFile> file_;
};

#endif  // LOG_FILE_H