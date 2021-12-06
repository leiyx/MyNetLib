#include "current_thread.h"
namespace CurrentThread {
__thread int t_cached_tid = 0;

void CacheTid() {
  if (t_cached_tid == 0) {
    // 通过linux系统调用，获取当前线程的tid值
    t_cached_tid = static_cast<pid_t>(::syscall(SYS_gettid));
  }
}
}  // namespace CurrentThread