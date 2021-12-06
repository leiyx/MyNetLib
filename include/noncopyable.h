/*
 * @Author: lei
 * @Description: 作为一个基类，被其他类继承。继承该类的类的对象可以正常的构造和析构，但不能拷贝构造和赋值操作
 * @FilePath: /MyNetLib/include/noncopyable.h
 */

#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

class noncopyable {
 public:
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

#endif  // NONCOPYABLE_H