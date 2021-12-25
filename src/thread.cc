#include "thread.h"

#include <semaphore.h>

#include "current_thread.h"

std::atomic_int Thread::num_created_{0};

Thread::Thread(ThreadFunc func, const std::string& name)
    : func_(std::move(func)),
      name_(name),
      tid_(0),
      started_(false),
      joined_(false) {
  SetDefaultName();
}
Thread::~Thread() {
  if (started_ && !joined_)
    thread_->detach();  // 这种情况，线程的资源只能留给系统去回收
}

void Thread::Start() {
  started_ = true;
  sem_t sem;
  sem_init(&sem, false, 0);

  // 开启线程
  thread_ = std::shared_ptr<std::thread>(new std::thread([&]() {
    // 获取线程的tid值
    tid_ = CurrentThread::Tid();
    sem_post(&sem);
    // 开启一个新线程，专门执行该线程函数
    func_();
  }));

  // 这里必须等待获取上面新创建的线程的tid值;
  // 比如线程1创建的线程2
  // ，在线程2tid可能还没设置好，这样如果线程1使用线程2tid就会出错;
  sem_wait(&sem);
}
void Thread::Join() {
  joined_ = true;
  thread_->join();
}

void Thread::SetDefaultName() {
  int num = ++num_created_;
  if (name_.empty()) {
    char buf[32] = {0};
    snprintf(buf, sizeof buf, "Thread %d", num);
    name_ = buf;
  }
}