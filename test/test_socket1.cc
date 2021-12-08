// socket服务端
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "socket.h"

void Test() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) LOG_FATAL << "socket() error";
  Socket s(fd);
  s.SetKeepAlive(true);
  s.SetReuseAddr(true);
  s.SetReusePort(true);
  InetAddr addr(10001);
  LOG_INFO << "服务端socket地址：" << addr.ToIpPort();
  s.Bind(addr);
  s.Listen();
  InetAddr peer_addr;
  int connfd = s.Accept(&peer_addr);
  std::cout << "connfd=" << connfd << std::endl;
  LOG_INFO << "客户端socket地址：" << peer_addr.ToIpPort();
  ::close(connfd);
}
int main() {
  Test();
  return 0;
}
