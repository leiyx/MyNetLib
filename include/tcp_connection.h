/*
 * @Author: lei
 * @Description: 一条TCP连接的抽象
 * @FilePath: /MyNetLib/include/tcp_connection.h
 */
#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <any>
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_set>

#include "buffer.h"
#include "callbacks.h"
#include "event_loop.h"
#include "inet_addr.h"
#include "logger.h"
#include "time_stamp.h"

class Channel;
class Socket;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const InetAddr& local_addr,
                const InetAddr& peer_addr, int fd, const std::string& name);
  ~TcpConnection();

  EventLoop* GetLoop() const { return loop_; }
  const std::string Name() const { return name_; }
  const InetAddr& LocalAddr() const { return local_addr_; }
  const InetAddr& PeerAddr() const { return peer_addr_; }
  bool Connected() const { return state_ == kConnected; }

  //设置回调函数
  void SetConnectionCallback(const ConnectionCallback& cb) {
    connection_callback_ = std::move(cb);
  }
  void SetMessageCallback(const MessageCallback& cb) {
    message_callback_ = std::move(cb);
  }
  void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
    write_complete_callback_ = std::move(cb);
  }
  void SetCloseCallback(const CloseCallback& cb) {
    close_callback_ = std::move(cb);
  }

  void Send(const std::string& buf);
  void Send(const char* buf, int len);
  void ShutDown();
  void ConnectionEstablished();
  void ConnectionDestroyed();

  void SetContext(std::any a) { context_ = a; }
  std::any GetContext() { return context_; }

  Timer* AddTimerTask(int32_t repeated_times, int64_t interval,
                      TimeEventCallback callback) {
    return loop_->RunAfter(repeated_times, interval, callback);
  }

 private:
  void HandleRead(TimeStamp receive_time);
  void HandleWrite();
  void HandleClose();
  void HandleError();

  void SendInLoop(const void* message, size_t len);
  void ShutdownInLoop();

 private:
  enum State {
    kConnecting,
    kConnected,
    kDisconnecting,
    kDisconnected,
  };
  void SetState(State state) { state_ = state; }

  EventLoop* loop_;

  std::atomic_int state_;
  const std::string name_;

  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;

  const InetAddr local_addr_;
  const InetAddr peer_addr_;

  Buffer input_buffer_;
  Buffer output_buffer_;

  // 处理三个半事件（连接建立、连接断开、消息到达、消息发送）的回调,由TcpServer设置
  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;

  // 用于绑定TcpServer的一个处理函数，因为TcpConnection由TcpServer管理
  CloseCallback close_callback_;
  std::any context_;  // C++17
};
// TODO: 为什么mainloop没有TcpConnection？
// TODO: 高水位？？？
// TODO: enable_share_from_this

#endif  // TCP_CONNECTION_H