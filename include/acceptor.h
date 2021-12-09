/*
 * @Author: lei
 * @Description: 被动连接的抽象
 * @FilePath: /MyNetLib/include/acceptor.h
 */
#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <functional>

#include "channel.h"
#include "inet_addr.h"
#include "socket.h"
// acceptor运行在mainloop
class EventLoop;
class Acceptor {
 public:
  using NewConnectionCallback =
      std::function<void(int connfd, InetAddr peer_addr)>;
  Acceptor(EventLoop* loop, const InetAddr& listen_addr, bool reuse_port);
  ~Acceptor();
  void SetNewConnectionCallback(const NewConnectionCallback& cb) {
    new_connection_callback_ = std::move(cb);
  }
  bool Listened() const { return listenning_; }
  void Listen();

 private:
  void HandleRead();  // accept_socket_的读事件回调
 private:
  EventLoop* loop_;  // 用户定义的那个EventLoop,即mainloop
  Socket accept_socket_;
  Channel accept_channel_;
  bool listenning_;
  NewConnectionCallback
      new_connection_callback_;  // 将新建立的connfd打包成channel，分发给相应的EventLoop;该回调由Tcpserver给出
};
#endif  // ACCEPTOR_H