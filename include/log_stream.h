/*
 * @Author: lei
 * @Description: LogStream类重载<<运算符，将各种信息追加到其内部的定长buffer里
 * @FilePath: /MyNetLib/include/log_stream.h
 */
#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include <string.h>

#include <string>

const int kSmallBufferSize = 4000;
const int kBigBufferSize = 4000 * 1000;

// 定长buffer
template <int SIZE>
class FixedBuffer {
 public:
  FixedBuffer() : cur_(data_) {}
  ~FixedBuffer() = default;

  int Avail() { return static_cast<int>(End() - cur_); }
  const char* GetData() const { return data_; }
  int Length() const { return static_cast<int>(cur_ - data_); }
  std::string ToString() const { return string(data_, Length()); }
  void Reset() { cur_ = data_; }
  void Bzero() { bzero(data_, sizeof data_); }

  void append(const char* buf, size_t len) {
    if (static_cast<size_t>(Avail()) > len) {
      memcpy(cur_, buf, len);
      HasWriten(len);
    }
  }

 private:
  const char* End() const { return data_ + sizeof data_; }
  void HasWriten(size_t len) { cur_ += len; }

 private:
  char data_[SIZE] = {' '};
  char* cur_;
};

// 将日志消息的各个部分，写到buffer中
class LogStream {
 public:
  using Buffer = FixedBuffer<kSmallBufferSize>;

  LogStream() = default;
  ~LogStream() = default;

  // 对各种类型重载<<运算符
  LogStream& operator<<(int32_t t);
  LogStream& operator<<(int64_t t);
  LogStream& operator<<(float);
  LogStream& operator<<(double);
  LogStream& operator<<(bool);
  LogStream& operator<<(char);
  LogStream& operator<<(const char* str);
  LogStream& operator<<(const std::string& str);

  void Append(const char* data, int len) { buffer_.append(data, len); }
  const Buffer& GetBuffer() const { return buffer_; }
  void ResetBuffer() { buffer_.Reset(); }

 private:
  Buffer buffer_;
};

#endif  // LOG_STREAM_H