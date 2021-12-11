#include "acceptor.h"

#include "socket_utils.h"

static int CreateNonblocking() {
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (sockfd < 0) LOG_FATAL << "listen socket: " << sockfd << "create error ";
  LOG_INFO << "fds" << sockfd;
  return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddr& ListenAddr, bool reuse_port)
    : loop_(loop),
      accept_socket_(CreateNonblocking()),
      accept_channel_(loop, accept_socket_.GetFd()),
      listenning_(false) {
  int fd = accept_socket_.GetFd();
  socket_utils::SetReuseAddr(fd, reuse_port);
  socket_utils::SetReusePort(fd, reuse_port);
  accept_socket_.Bind(ListenAddr);

  accept_channel_.SetReadEventCallback(
      std::bind((&Acceptor::HandleRead), this));
}
Acceptor::~Acceptor() {
  accept_channel_.DisableAll();
  accept_channel_.Remove();
}

// Tcpser::Start -> Acceptor::Listen
void Acceptor::Listen() {
  listenning_ = true;
  accept_socket_.Listen();
  accept_channel_.EnableRead();
  LOG_INFO << "Acceptor::listen from now on";
}

// listenfd有读事件，即有新连接到来,执行注册在该accept_channel_的读事件回调
void Acceptor::HandleRead() {
  InetAddr peeraddr;
  int connfd = accept_socket_.Accept(&peeraddr);
  if (connfd >= 0) {
    if (new_connection_callback_) {
      new_connection_callback_(connfd, peeraddr);
    } else
      ::close(connfd);
  } else {
    LOG_ERROR << "listen socket: " << accept_socket_.GetFd() << "create error ";
    if (errno = EMFILE) {
      LOG_ERROR << "listen socket: " << accept_socket_.GetFd()
                << "create error ";
    }
  }
}
