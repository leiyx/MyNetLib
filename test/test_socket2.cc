// socket客户端

#include <iostream>
#include <string>

#include "socket.h"
#include "socket_utils.h"

void Test() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) LOG_FATAL << "socket() error";
  Socket s(fd);
  socket_utils::SetKeepAlive(fd, true);
  socket_utils::SetReuseAddr(fd, true);
  socket_utils::SetReusePort(fd, true);
  InetAddr cliaddr(10004);
  InetAddr seraddr("127.0.0.1", 10001);
  LOG_INFO << "客户端socket地址：" << cliaddr.ToIpPort();
  LOG_INFO << "服务端socket地址：" << seraddr.ToIpPort();
  s.Connect(s.GetFd(), &seraddr);  // addr为服务端地址
}
int main() {
  Test();
  return 0;
}
