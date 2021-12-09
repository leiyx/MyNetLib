/*
 * @Author: lei
 * @Description: 对socket基本api和选项的包装
 * @FilePath: /MyNetLib/include/socket.h
 */
#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>

#include "inet_addr.h"
#include "logger.h"
#include "noncopyable.h"

class Socket : noncopyable {
 public:
  explicit Socket(int sockfd) : fd_(sockfd) {}
  ~Socket();

  int GetFd() const { return fd_; }

  // 对socket基本api的包装
  void Bind(const InetAddr& localaddr);
  void Listen();
  int Accept(InetAddr* peeraddr, bool nonblock = true);
  void Connect(int server_listen_fd, InetAddr* local_addr);
  void ShutdownWrite();

  // 一组socket选项设置，包装了setsockopt函数
  void SetReuseAddr(bool on = true);
  void SetReusePort(bool on = true);
  void SetKeepAlive(bool on = true);

  static const sockaddr_in GetLocalAddr(int sockfd);
  static const sockaddr_in GetPeerAddr(int sockfd);
  static bool SetNonblocking(int fd, bool on = true);

 private:
  const int fd_;
};

#endif  // SOCKET_H