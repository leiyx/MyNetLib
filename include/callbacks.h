/*
 * @Author: lei
 * @Description: 回调类型定义
 * @FilePath: /MyNetLib/include/callbacks.h
 */
#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>

class TimeStamp;
class EventLoop;

// 在event_loop.h中使用
using Functor = std::function<void()>;
using TimerEventCallback = std::function<void()>;

// 在logger.h中使用
//将数据从 应用程序buffer 写到 文件（未冲刷）
using OutputFunc = std::function<void(const char*, int)>;
//将文件内容冲刷到磁盘，内存 -〉 磁盘
using FlushFunc = std::function<void()>;

#endif  // CALLBACKS_H