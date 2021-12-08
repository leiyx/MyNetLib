#include "socket.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
Socket::~Socket() { ::close(fd_); }
void Socket::Bind(const InetAddr& localaddr) {
  if (::bind(fd_, localaddr.Get_Const_Sockaddr_ptr(), sizeof(sockaddr_in)) != 0)
    LOG_FATAL << "bind sockfd:" << fd_ << " error!";
}
void Socket::Listen() {
  if (0 != ::listen(fd_, 1024))
    LOG_FATAL << "listen sockfd:" << fd_ << " error!";
}

/**
 * @description: 对accept系统调用的封装
 * @param {InetAddr*} peeraddr:传出参数，用于保存客户端的socket地址
 * @return {int} 返回新建立的connfd值
 */
int Socket::Accept(InetAddr* peeraddr)  //输出参数
{
  sockaddr_in addr;
  socklen_t len = sizeof(sockaddr);
  int connfd =
      ::accept4(fd_, (sockaddr*)&addr, &len, SOCK_CLOEXEC | SOCK_NONBLOCK);
  if (connfd > 0) {
    peeraddr->SetAddr(addr);
    LOG_INFO << "accept new connection from " << peeraddr->ToIpPort().c_str();
  } else
    LOG_FATAL << "accept sockfd:" << fd_ << " error!";
  return connfd;
}
void Socket::Connect(int local_fd, InetAddr* ser_addr) {
  int ret =
      ::connect(local_fd, ser_addr->Get_Const_Sockaddr_ptr(), sizeof(sockaddr));
  // LOG_FATAL << "connect to listenfd: " << local_fd << " failed!";
}
void Socket::ShutdownWrite() {
  if (::shutdown(fd_, SHUT_WR) < 0)
    LOG_ERROR << "shutdownWrite sockfd: " << fd_ << " error!";
}

void Socket::SetReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::SetReusePort(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}

void Socket::SetKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}

const sockaddr_in Socket::GetLocalAddr(int sockfd) {
  struct sockaddr_in local_addr;
  bzero(&local_addr, sizeof local_addr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof local_addr);
  if (::getsockname(sockfd, (struct sockaddr*)&local_addr, &addrlen) < 0) {
    LOG_ERROR << "sockets::GetLocalAddr";
  }
  return local_addr;
}

const sockaddr_in Socket::GetPeerAddr(int sockfd) {
  struct sockaddr_in peer_addr;
  bzero(&peer_addr, sizeof peer_addr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peer_addr);
  if (::getpeername(sockfd, (struct sockaddr*)&peer_addr, &addrlen) < 0) {
    LOG_ERROR << "sockets::GetPeerAddr";
  }
  return peer_addr;
}