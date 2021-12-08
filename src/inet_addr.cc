#include "inet_addr.h"

#include <cstring>

InetAddr::InetAddr(const std::string& ip, uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}
InetAddr::InetAddr(uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  addr_.sin_addr.s_addr = INADDR_ANY;
}
InetAddr::InetAddr() {
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(10001);
  addr_.sin_addr.s_addr = INADDR_ANY;
}
std::string InetAddr::ToIp() const {
  char buf[64] = {0};
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
  return buf;
}
std::string InetAddr::ToIpPort() const {
  char buf[64] = {0};
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
  size_t end = strlen(buf);
  uint16_t port = ntohs(addr_.sin_port);
  sprintf(buf + end, ":%u", port);
  return buf;
}
uint16_t InetAddr::ToPort() const { return ntohs(addr_.sin_port); }