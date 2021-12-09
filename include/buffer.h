/*
 * @Author: lei
 * @Description: 变长缓冲区
 * @FilePath: /MyNetLib/include/buffer.h
 */
#ifndef BUFFER_H
#define BUFFER_H

#include <algorithm>
#include <string>
#include <vector>

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
class Buffer {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
      : buffer_(kInitialSize),
        read_index_(kCheapPrepend),
        write_index_(kCheapPrepend) {}

  size_t ReadableBytes() const { return write_index_ - read_index_; }
  size_t WriteableBytes() const { return buffer_.size() - write_index_; }
  size_t PrependableBytes() const { return read_index_; }

  const char* BeginRead() const { return Begin() + read_index_; }
  char* BeginRead() { return Begin() + read_index_; }
  char* BeginWrite() { return Begin() + write_index_; }
  const char* BeginWrite() const { return Begin() + write_index_; }

  void Retrive(size_t len) {
    if (len < ReadableBytes()) {
      read_index_ += len;
    } else
      RetriveAll();
  }
  void RetriveAll() { read_index_ = write_index_ = kCheapPrepend; }
  std::string RetriveAsString(size_t len) {
    std::string result(BeginRead(), len);
    Retrive(len);
    return result;
  }
  std::string RetriveAllAsString() { return RetriveAsString(ReadableBytes()); }

  void EnsureWriteableBytes(size_t len) {
    if (WriteableBytes() < len) {
      MakeSpace(len);
    }
  }
  //一组append接口
  void Append(const char* data, size_t len) {
    EnsureWriteableBytes(len);
    std::copy(data, data + len, BeginWrite());
  }

  //分散读、分散写
  ssize_t ReadFd(int fd, int* saveErrno);
  ssize_t WriteFd(int fd, int* saveErrno);

 private:
  std::vector<char> buffer_;
  size_t read_index_;
  size_t write_index_;

  char* Begin() { return &*buffer_.begin(); }
  const char* Begin() const { return &*buffer_.begin(); }

  void MakeSpace(size_t len) {
    // 空间不足
    if (WriteableBytes() + PrependableBytes() < len + kCheapPrepend) {
      buffer_.resize(write_index_ + len);
    }
    // 空间足够,但需要整理
    else {
      size_t readable = ReadableBytes();
      std::copy(Begin() + read_index_, Begin() + write_index_,
                Begin() + kCheapPrepend);
      read_index_ = kCheapPrepend;
      write_index_ = read_index_ + readable;
    }
  }
};

#endif  // BUFFER_H