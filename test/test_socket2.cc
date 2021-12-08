// socket客户端
#include <sys/socket.h>

#include <iostream>
#include <string>

#include "socket.h"

void Test() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) LOG_FATAL << "socket() error";
  Socket s(fd);
  s.SetKeepAlive(true);
  s.SetReuseAddr(true);
  s.SetReusePort(true);
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
