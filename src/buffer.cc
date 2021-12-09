#include "buffer.h"

#include <sys/uio.h>
#include <unistd.h>

#include "logger.h"
/**
 * @description: 从fd读数据到缓冲区,分散读,使用readv系统调用
 * @param {int} fd
 * @param {int*} saveErrno:传出参数，保存错误号
 * @return {ssize_t} 返回读到的数据的字节数
 */
ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
  char extrabuf[65535] = {0};  // 栈上预备64k空间
  struct iovec vec[2];
  const size_t writeable = WriteableBytes();  // 先缓存可写字节数

  vec[0].iov_base = BeginWrite();
  vec[0].iov_len = writeable;

  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;

  const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;

  const ssize_t n = ::readv(fd, vec, iovcnt);
  if (n < 0) {
    *saveErrno = errno;
    LOG_ERROR << "read " << fd << " error!";
  } else if (n < writeable)  // 只写入了buffer_
  {
    write_index_ += n;
  } else  // buffer_和栈都写入了
  {
    write_index_ = buffer_.size();
    Append(extrabuf, n - writeable);
  }
  return n;
}

// 不需要使用writev，要发送的数据肯定都在缓冲区里
ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
  ssize_t n = ::write(fd, BeginRead(), ReadableBytes());
  if (n < 0) {
    *saveErrno = errno;
    LOG_ERROR << "write " << fd << " error!";
  }
  return n;
}