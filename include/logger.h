/*
 * @Author: lei
 * @Description: 负责打印日志
 * @FilePath: /MyNetLib/include/logger.h
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <assert.h>
#include <callbacks.h>
#include <string.h>
#include <sys/time.h>

#include <functional>
#include <string>

#include "async_logging.h"
#include "log_file.h"
#include "log_stream.h"
#include "time_stamp.h"
class Logger {
 public:
  enum LogLevel {
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
  };

  Logger(const char* file, int line) : impl_(INFO, file, line) {}

  Logger(const char* file, int line, LogLevel level, const char* func)
      : impl_(level, file, line) {
    impl_.stream_ << func << ' ';
  }
  Logger(const char* file, int line, LogLevel level)
      : impl_(level, file, line) {}

  Logger(const char* file, int line, bool toAbort)
      : impl_(toAbort ? FATAL : ERROR, file, line) {}

  ~Logger();

  LogLevel GetLogLevel();
  void SetLogLevel(Logger::LogLevel);

  static void SetOutputFunc(
      OutputFunc);  //将日志信息从 应用程序buffer 写到 文件对应的缓冲区;
  static void SetFlushFunc(
      FlushFunc);  //将日志信息从 文件对应的缓冲区（fwrite）冲刷到 磁盘;

  LogStream& Stream() { return impl_.stream_; }

  // 内部实现类，其功能：组装一条日志消息，为 输出内容 加上
  // 线程id、时间戳、日志级别、文件名、行号信息;
  class Impl {
   public:
    Impl(Logger::LogLevel level, const char* filename, int line);
    ~Impl() = default;
    void FormatTime();
    void Finish();

   public:
    // LogStream类功能：重载operator<<，将日志内容写到buffer中;
    LogStream stream_;

    LogLevel level_;        // 日志等级;
    int line_;              // __LINE__;
    const char* filename_;  // __FILE__;
  };

 private:
  Impl impl_;
};

#define LOG_TRACE                          \
  if (Logger::LogLevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE, __FUNCTION__).Stream()
#define LOG_DEBUG                          \
  if (Logger::LogLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __FUNCTION__).Stream()
#define LOG_INFO \
  if (Logger::LogLevel() <= Logger::INFO) Logger(__FILE__, __LINE__).Stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).Stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).Stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).Stream()

#endif  // LOGGER_H