#include <unistd.h>

#include <cstdio>

#include "current_thread.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
void print(EventLoop* p = NULL) {
  printf("main(): pid = %d, tid = %d, loop = %p\n", getpid(),
         CurrentThread::Tid(), p);
}

void init(EventLoop* p) {
  printf("init(): pid = %d, tid = %d, loop = %p\n", getpid(),
         CurrentThread::Tid(), p);
}

int main() {
  print();

  EventLoop loop;
  loop.RunAfter(0, 10000, std::bind(&EventLoop::Quit, &loop));

  {
    printf("Single thread %p:\n", &loop);
    EventLoopThreadPool model(&loop, "single");
    model.SetThreadNum(0);
    model.Start(init);
  }

  {
    printf("Another thread:\n");
    EventLoopThreadPool model(&loop, "another");
    model.SetThreadNum(1);
    model.Start(init);
    EventLoop* next_loop = model.GetNextLoop();
    ::sleep(3);
  }

  {
    printf("Three threads:\n");
    EventLoopThreadPool model(&loop, "three");
    model.SetThreadNum(3);
    model.Start(init);
    EventLoop* next_loop = model.GetNextLoop();
    next_loop->RunInLoop(std::bind(print, next_loop));
  }

  loop.Loop();
}
