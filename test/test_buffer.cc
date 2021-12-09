#include <algorithm>
#include <string>

#include "buffer.h"
#include "logger.h"
#include "socket.h"

// 实现简单的回射
void Test1() {
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
  int connfd = s.Accept(&peer_addr, false);
  std::cout << "connfd=" << connfd << std::endl;
  LOG_INFO << "客户端socket地址：" << peer_addr.ToIpPort();

  Buffer buffer(2048);

  int err;
  buffer.ReadFd(connfd, &err);
  buffer.WriteFd(connfd, &err);
  ::close(connfd);
}
int main() {
  Test1();
  return 0;
}