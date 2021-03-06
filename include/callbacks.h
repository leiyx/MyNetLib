/*
 * @Author: lei
 * @Description: 回调类型定义
 * @FilePath: /MyNetLib/include/callbacks.h
 */
#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>
#include <memory>

class TcpConnection;
class TimeStamp;
class EventLoop;
class Buffer;

// 在event_loop.h中使用
using Functor = std::function<void()>;
using TimerEventCallback = std::function<void()>;

// 在event_loop_thread_pool.h使用
using ThreadInitCallback = std::function<void(EventLoop *)>;

// 在logger.h中使用
//将数据从 应用程序buffer 写到 文件（未冲刷）
using OutputFunc = std::function<void(const char *, int)>;
//将文件内容冲刷到磁盘，内存 -〉 磁盘
using FlushFunc = std::function<void()>;

// tcp_connection.h
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback =
    std::function<void(const TcpConnectionPtr &, Buffer *, TimeStamp)>;

// tcp_server.h
using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
#endif  // CALLBACKS_H