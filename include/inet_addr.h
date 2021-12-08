/*
 * @Author: lei
 * @Description: 对socket地址的封装
 * @FilePath: /MyNetLib/include/inet_addr.h
 */
#ifndef INET_ADDR_H
#define INET_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

class InetAddr {
 public:
  explicit InetAddr(const sockaddr_in& addr) : addr_(addr) {}
  InetAddr(const std::string& ip, uint16_t port);
  InetAddr(uint16_t port);
  InetAddr();  // 默认地址0.0.0.0:10001
  ~InetAddr() = default;

  std::string ToIp() const;
  std::string ToIpPort() const;
  uint16_t ToPort() const;

  void SetAddr(const sockaddr_in& addr) { addr_ = addr; }
  sockaddr_in GetSockaddr_in() const { return addr_; }

  // 注意：常对象只能调用常函数,非常对象能调用普通函数和常函数；
  const sockaddr* Get_Const_Sockaddr_ptr() const {
    return (const sockaddr*)&addr_;
  }
  sockaddr* Get_Sockaddr_ptr() const { return (sockaddr*)&addr_; }
  const socklen_t SockaddrLen() const { return sizeof(sockaddr); }

 private:
  struct sockaddr_in addr_;
};

#endif  // INET_ADDR_H