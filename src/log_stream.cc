#include "log_stream.h"

LogStream& LogStream::operator<<(int64_t t) {
  std::string s = std::to_string(t);
  buffer_.append(s.c_str(), s.size());
  return *this;
}
LogStream& LogStream::operator<<(int32_t t) {
  std::string s = std::to_string(t);
  buffer_.append(s.c_str(), s.size());
  return *this;
}
LogStream& LogStream::operator<<(float t) {
  std::string s = std::to_string(t);
  buffer_.append(s.c_str(), s.size());
  return *this;
}
LogStream& LogStream::operator<<(double t) {
  std::string s = std::to_string(t);
  buffer_.append(s.c_str(), s.size());
  return *this;
}
LogStream& LogStream::operator<<(bool t) {
  if (t)
    buffer_.append("true ", 5);
  else
    buffer_.append(" false ", 6);
  return *this;
}
LogStream& LogStream::operator<<(char t) {
  char p[1];
  p[0] = t;
  *this << p;
  return *this;
}
LogStream& LogStream::operator<<(const char* str) {
  if (str) {
    buffer_.append(str, strlen(str));
    // sizeof是编译期计算,但这里不可以使用sizeof，因为sizeof(str)结果将始终是8，str是一个指向字符数组的指针
  } else {
    buffer_.append("(null)", 6);
  }
  return *this;
}

LogStream& LogStream::operator<<(const std::string& str) {
  buffer_.append(str.c_str(), str.size());
  return *this;
}

// TODO: 重载更多类型