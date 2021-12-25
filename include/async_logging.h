/*
 * @Author: lei
 * @Description: 日志的异步打印类（日志打印到文件）
 * @FilePath: /MyNetLib/include/async_logging.h
 */
#ifndef ASYNC_LOGGING_H
#define ASYNC_LOGGING_H

#include <cassert>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "log_stream.h"
#include "thread.h"

// 一个或多个前端线程（业务线程）-- blockingqueue -- 一个日志线程（后端线程）
// 异步日志写入：
// 1. 让前端线程（业务线程）不被写日志操作阻塞，能一直写日志
// 2. 让日志线程，每次写文件写尽可能多的内容，即不被频繁唤醒；

class AsyncLogging {
 public:
  using Buffer = FixedBuffer<kBigBufferSize>;  // 可配置FixedBuffer大小
  using BufferPtr = std::unique_ptr<Buffer>;
  using BufferPtrVec = std::vector<std::unique_ptr<Buffer>>;

  AsyncLogging(const std::string& filename, long roll_size,
               int flush_interval = 3);
  ~AsyncLogging();
  void Start();
  void Stop();
  void Append(const char* msg, int len);
  // 不再需要设置FlushFunc，即本类不需要Flush函数，因为单开了另一个线程去写日志到文件，每写完一次就冲刷到磁盘

  Thread thread_;

 private:
  void ThreadFunc();

 private:
  bool running_;

  const std::string filename_;
  int flush_interval_;
  long roll_size_;

  // 两个前端buffer和blockingqueue
  BufferPtr cur_buffer_;
  BufferPtr next_buffer_;
  BufferPtrVec buffer_to_write_;

  // 一个互斥量和条件变量用于互斥与同步，buffer_to_write_对多个前端线程而言是临界资源
  std::mutex mutex_;
  std::condition_variable cond_;
};

#endif  // ASYNC_LOGGING_H