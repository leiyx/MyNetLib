/*
 * @Author: lei
 * @Description: 回调类型定义
 * @FilePath: /MyMuduo/include/callbacks.h
 */
#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>
#include <memory>

class Buffer;
class TimeStamp;
class TcpConnection;
class EventLoop;

// tcp_server.h中
using ThreadInitCallback = std::function<void(EventLoop*)>;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
// tcp_connection.h
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback =
    std::function<void(const TcpConnectionPtr&, Buffer*, TimeStamp)>;

using TimerEventCallback = std::function<void()>;

using OutputFunc = std::function<void(
    const char*, int)>;  //将数据从 应用程序buffer 写到 文件（未冲刷）
using FlushFunc = std::function<void()>;  //将文件内容冲刷到磁盘，内存 -〉 磁盘

#endif  // CALLBACKS_H