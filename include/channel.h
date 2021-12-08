/*
 * @Author: lei
 * @Description:
 * Reactor模型中的统一事件源和事件处理器，对fd及其监听事件进行包装;
 * @FilePath: /MyNetLib/include/channel.h
 */
#ifndef CHANNEL_H
#define CHANNEL_H
#include <functional>
#include <memory>

#include "noncopyable.h"
class TimeStamp;
class EventLoop;

// 一个Channel对象对应一个文件描述符，负责该fd上IO事件的注册及响应
// 一个EventLoop对象拥有多个Channel对象，一个Poller对象，EventLoop对象作为Channel和Poller的桥梁;
class Channel : noncopyable {
 public:
  using ReadEventCallback = std::function<void(TimeStamp)>;
  using WriteEventCallback = std::function<void()>;
  using ConnectionCloseCallback = std::function<void()>;
  using ErrorCallback = std::function<void()>;

  Channel(EventLoop* loop, int fd);
  ~Channel() = default;

  void HandleEvent(TimeStamp received_time);

  bool IsNoneEvent() const { return events_ == kNoneEvent; }
  bool IsWriting() const { return events_ & kWriteEvent; }
  bool IsReading() const { return events_ & kReadEvent; }
  void EnableRead() {
    events_ |= kReadEvent;
    Update();
  }
  void DisableRead() {
    events_ &= ~kReadEvent;
    Update();
  }
  void EnableWrite() {
    events_ |= kWriteEvent;
    Update();
  }
  void DisableWrite() {
    events_ &= ~kWriteEvent;
    Update();
  }
  void DisableAll() {
    events_ = kNoneEvent;
    Update();
  }

  void SetReadEventCallback(ReadEventCallback cb) {
    read_event_callback_ = std::move(cb);
  }
  void SetWriteEventCallback(WriteEventCallback cb) {
    write_event_callback_ = std::move(cb);
  }
  void SetConnectionCloseCallback(ConnectionCloseCallback cb) {
    connection_close_callback_ = std::move(cb);
  }
  void SetErrorCallback(ErrorCallback cb) { error_callback_ = std::move(cb); }

  int Fd() const { return fd_; }
  int Events() const { return events_; }
  int Index() const { return index_; }
  EventLoop* OwnerLoop() const { return loop_; }

  //由Poller调用该接口，使Channel接受Poller分发回来的事件
  void Set_Revents(int received_events) { revents_ = received_events; }
  void Set_Index(int idx) { index_ = idx; }  // 由Poller调用该接口，设置状态

  // 防止当channel被手动remove掉，channel还在执行回调操作
  void Tie(const std::shared_ptr<void>&);  // 将弱智能指针绑定到shared_ptr上

  void Remove();
  //从Poller中内核监听列表中删除;调用层次：Channel::remove -->
  // EventLoop::removeChannel --> Poller::removeChannel

 private:
  void Update();  // 更新感兴趣事件到内核注册表上，调用层次与Remove同
  void HandleEventWithGuard(TimeStamp received_time);

 private:
  EventLoop* loop_;
  const int fd_;
  int events_;
  int revents_;
  int index_;  // 用于Poller中

  //弱智能指针，为了防止手动remove该channle后，
  // 还在执行该channel的回调，用于handleEvent中;
  std::weak_ptr<void> tie_;
  bool tied_;

  static int kReadEvent;
  static int kWriteEvent;
  static int kNoneEvent;

  ReadEventCallback read_event_callback_;
  WriteEventCallback write_event_callback_;
  ConnectionCloseCallback connection_close_callback_;
  ErrorCallback error_callback_;
};

#endif  // CHANNEL_H