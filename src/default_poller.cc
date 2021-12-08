#include "epoll_poller.h"
#include "poll_poller.h"
#include "poller.h"
// 一个EventLoop 与 一个Poller 对应，该函数被EventLoop调用,
// 类型向上转型，安全;
Poller* Poller::NewDefaultPoller(EventLoop* loop) {
#ifdef USE_POLL
  return new PollPoller(loop);
#else
  return new EpollPoller(loop);
#endif
  return nullptr;
}