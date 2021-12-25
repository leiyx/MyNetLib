/*
 * @Author: lei
 * @Description: 用户编写服务端程序使用的类
 * @FilePath: /MyNetLib/include/tcp_server.h
 */
#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "acceptor.h"
#include "buffer.h"
#include "callbacks.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "inet_addr.h"
#include "logger.h"
#include "noncopyable.h"
#include "tcp_connection.h"

class TcpServer {
 public:
  enum Option {
    kNoReusePort,
    kReusePort,
  };

  TcpServer(EventLoop* loop, const InetAddr& listen_addr,
            const std::string& name_arg, Option option = kReusePort);
  ~TcpServer();

  void SetThreadInitCallback(const ThreadInitCallback& cb) {
    thread_init_callback_ = std::move(cb);
  }
  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_callback_ = std::move(cb);
  }
  void SetMessageCallback(const MessageCallback& cb) {
    message_callback_ = std::move(cb);
  }
  void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
    write_complete_callback_ = std::move(cb);
  }

  void SetThreadNum(int num);  //设置subloop的数量

  void Start();  //开启服务器监听

  const std::string Name() const { return name_; }
  const std::string IpPort() const { return ip_port_; }

 private:
  // 该函数传给Acceptor中的new_connection_callback_
  void NewConnection(int sockfd, const InetAddr& peer_addr);

  // 该函数传给TcpConnection::HandleClose中的close_callback_
  void RemoveConnection(const TcpConnectionPtr& conn);
  void RemoveconnectionInLoop(const TcpConnectionPtr& conn);

 private:
  EventLoop* loop_;  // baseloop,即mainloop,由用户传入
  const std::string ip_port_;
  const std::string name_;
  int next_conn_id_;
  std::atomic_int started_;

  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> thread_pool_;

  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;
  ThreadInitCallback thread_init_callback_;

  ConnectionMap connection_maps;
};

#endif  // TCP_SERVER_H