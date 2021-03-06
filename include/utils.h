/*
 * @Author: lei
 * @Description: 工具类
 * @FilePath: /MyNetLib/include/utils.h
 */
#ifndef UTILS_H
#define UTILS_H

#include <sys/eventfd.h>

#include <chrono>
#include <functional>
#include <string>

#include "logger.h"

namespace utils {

// 使用C++11的chrono库测试函数运行时间
void TestTime(std::function<void()> f) {
  using namespace std::literals;  // 允许用 24h 、 1ms 、 1s 代替对应的
                                  // std::chrono::hours(24) 等待
  const std::chrono::time_point<std::chrono::steady_clock> start =
      std::chrono::steady_clock::now();
  // “现实生活”的替用写法会是：
  // const auto start = std::chrono::steady_clock::now();
  f();

  const auto end = std::chrono::steady_clock::now();
  std::cout << "f took "
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << "µs ≈ " << (end - start) / 1ms
            << "ms ≈ "                        // 几乎等价于以上形式，
            << (end - start) / 1s << "s.\n";  // 但分别使用毫秒和秒
  return;
}

int CreateEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_FATAL << "eventfd error: " << errno;
  }
  return evtfd;
}

}  // namespace utils

#endif  // UTILS_H