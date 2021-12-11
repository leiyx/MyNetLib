#include "socket_utils.h"
void socket_utils::SetReuseAddr(int fd, bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void socket_utils::SetReusePort(int fd, bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}

void socket_utils::SetKeepAlive(int fd, bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}

const sockaddr_in socket_utils::GetLocalAddr(int sockfd) {
  struct sockaddr_in local_addr;
  bzero(&local_addr, sizeof local_addr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof local_addr);
  if (::getsockname(sockfd, (struct sockaddr*)&local_addr, &addrlen) < 0) {
    LOG_ERROR << "sockets::GetLocalAddr";
  }
  return local_addr;
}

const sockaddr_in socket_utils::GetPeerAddr(int sockfd) {
  struct sockaddr_in peer_addr;
  bzero(&peer_addr, sizeof peer_addr);
  socklen_t addrlen = static_cast<socklen_t>(sizeof peer_addr);
  if (::getpeername(sockfd, (struct sockaddr*)&peer_addr, &addrlen) < 0) {
    LOG_ERROR << "sockets::GetPeerAddr";
  }
  return peer_addr;
}

bool socket_utils::SetNonblocking(int fd, bool on) {
  int old = fcntl(fd, F_GETFL);
  if (-1 == fcntl(fd, F_SETFL, old | O_NONBLOCK)) {
    LOG_ERROR << fd << "set nonbolock failed";
    return false;
  }
  return true;
}