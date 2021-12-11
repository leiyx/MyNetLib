#include "socket.h"

#include "socket_utils.h"
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
int Socket::Accept(InetAddr* peeraddr, bool nonblock)  //输出参数
{
  sockaddr_in addr;
  socklen_t len = sizeof(sockaddr);
  int connfd = ::accept4(fd_, (sockaddr*)&addr, &len, SOCK_CLOEXEC);
  if (nonblock) {
    socket_utils::SetNonblocking(connfd, true);
  }
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
}
void Socket::ShutdownWrite() {
  if (::shutdown(fd_, SHUT_WR) < 0)
    LOG_ERROR << "shutdownWrite sockfd: " << fd_ << " error!";
}