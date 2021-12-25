#include "async_logging.h"

#include <chrono>

#include "log_file.h"

AsyncLogging::AsyncLogging(const std::string& filename, long roll_size,
                           int flush_interval)
    : filename_(filename),
      roll_size_(roll_size),
      flush_interval_(flush_interval),
      running_(false),
      cur_buffer_(new Buffer),
      next_buffer_(new Buffer),
      thread_(std::bind(&AsyncLogging::ThreadFunc, this), "loggging thread") {
  cur_buffer_->Bzero();
  next_buffer_->Bzero();
  buffer_to_write_.reserve(16);
}
AsyncLogging::~AsyncLogging() {
  if (running_) Stop();
}
void AsyncLogging::Start() {
  running_ = true;
  thread_.Start();
}
void AsyncLogging::Stop() {
  running_ = false;
  cond_.notify_all();
  thread_.Join();
}
void AsyncLogging::Append(const char* msg, int len) {
  std::lock_guard<std::mutex> lk(mutex_);
  if (cur_buffer_->Avail() > len)
    cur_buffer_->append(msg, len);
  else {
    // 新建buffer，将有日志信息的旧buffer放入buffer_to_write_
    buffer_to_write_.push_back(std::move(cur_buffer_));
    if (next_buffer_)
      cur_buffer_ = std::move(next_buffer_);
    else
      // 始终让next_buffer为一块空闲区间，作为备用
      cur_buffer_.reset(new Buffer);
    cur_buffer_->append(msg, len);
    cond_.notify_all();
  }
}

void AsyncLogging::ThreadFunc() {
  // assert(running_ == true);
  // 异步写文件的目的地
  LogFile log_file_(filename_, roll_size_, false);
  // 两个后端buffer即 cur_buffer_ 和 next_buffer_
  BufferPtr new_buffer1(new Buffer);
  BufferPtr new_buffer2(new Buffer);
  new_buffer1->Bzero();
  new_buffer2->Bzero();
  // blockingqueue
  BufferPtrVec temp_buffer_vec;
  temp_buffer_vec.reserve(16);

  while (running_) {
    assert(new_buffer1 && new_buffer1->Length() == 0);
    assert(new_buffer2 && new_buffer2->Length() == 0);
    assert(temp_buffer_vec.empty());
    {
      std::unique_lock<std::mutex> lk(mutex_);

      // TODO:冲刷时间 是 摆设是吧？ 一顿操作下来，日志文件为空？？？
      /*while(buffer_to_write_.empty())
      {
              cond_.wait(lk);
      }*/
      if (buffer_to_write_.empty()) {
        using namespace std::chrono_literals;
        cond_.wait_for(lk, 5s);
        // 否则如果缓冲区一直不满，就一直不冲刷到磁盘，导致可能丢失有效日志信息，但这里设计不好，
        // 可改进,但是有可能出现虚假唤醒
      }

      buffer_to_write_.push_back(std::move(cur_buffer_));
      cur_buffer_ = std::move(new_buffer1);
      temp_buffer_vec.swap(
          buffer_to_write_);  // 减小锁的粒度，对temp_buffer_vec中的每一个buffer进行操作，就不需要再锁上了，属于空间换时间
      if (!next_buffer_) next_buffer_ = std::move(new_buffer2);
    }
    if (temp_buffer_vec.size() > 25)  // 自我保护，避免buffer无限增长
    {
      perror("日志消息太多了，给你丢弃一部分");
      temp_buffer_vec.erase(temp_buffer_vec.begin() + 2,
                            buffer_to_write_.end());
    }

    for (const auto& t : temp_buffer_vec) {
      log_file_.Append(t->GetData(), t->Length());
    }
    if (buffer_to_write_.size() > 2) {
      temp_buffer_vec.resize(2);
    }
    if (!new_buffer1) {
      new_buffer1 = std::move(temp_buffer_vec.back());
      temp_buffer_vec.pop_back();
      new_buffer1->Reset();
    }
    if (!new_buffer2) {
      new_buffer2 = std::move(temp_buffer_vec.back());
      temp_buffer_vec.pop_back();
      new_buffer2->Reset();
    }
    temp_buffer_vec.clear();
    log_file_.Flush();
  }
  log_file_.Flush();
}