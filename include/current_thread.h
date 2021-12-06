/*
 * @Author: lei
 * @Description: 获取当前线程的线程id
 * @FilePath: /MyNetLib/include/current_thread.h
 */

#ifndef CURRENT_THREAD_H
#define CURRENT_THREAD_H

#include <sys/syscall.h>
#include <unistd.h>

#include <string>

namespace CurrentThread {
extern __thread int t_cached_tid;
// __thread等同于thread_local,
// 这虽然是一个全局变量，但会在每一个线程里存一个副本;
void CacheTid();

inline int Tid() {
  if (__builtin_expect(t_cached_tid == 0, 0))  //语句优化
  {
    CacheTid();
  }
  return t_cached_tid;
}
static std::string TidAsString() {
  int tid = CurrentThread::Tid();
  return std::to_string(tid);
}
}  // namespace CurrentThread

#endif  // CURRENT_THREAD_H