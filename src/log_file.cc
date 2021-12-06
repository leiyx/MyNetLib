#include "log_file.h"

#include "assert.h"
// ----------AppendFile成员函数实现----------------
AppendFile::AppendFile(std::string& filename)
    : fp_(::fopen(filename.c_str(), "ae")), written_bytes_(0) {
  assert(fp_);
  ::setbuffer(fp_, buffer_,
              sizeof buffer_);  // c库函数，设置文件指针fp_的对应缓冲区;
}
AppendFile::~AppendFile() { ::fclose(fp_); }
size_t AppendFile::Write(const char* log_line, size_t len) {
  return ::fwrite_unlocked(log_line, 1, len, fp_);
}
void AppendFile::Append(const char* log_line, size_t len) {
  size_t written = 0;
  // 使用while避免一次Write写不完的情况
  while (len != written) {
    size_t remain = len - written;
    size_t n = Write(log_line + written, remain);
    if (n != remain) {
      int err = ferror(fp_);
      if (err) {
        perror("error!");
        break;
      }
    }
    written += n;
  }
  written_bytes_ += written;
}
void AppendFile::Flush() { ::fflush(fp_); }

// ----------LogFile成员函数实现----------------
LogFile::LogFile(const std::string& base_name, long roll_size, bool thread_safe,
                 int flush_interval, int check_every_n)
    : base_name_(base_name),
      roll_size_(roll_size),
      thread_safe_(thread_safe),
      mutex_(),
      check_every_n_(check_every_n),
      flush_interval_(flush_interval),
      count_(0),
      start_of_period_(0),
      last_flush_(0),
      last_roll_(0) {
  RollFile();
}
LogFile::~LogFile() = default;

void LogFile::Append(const char* log_line, int len) {
  if (thread_safe_) {
    std::lock_guard<std::mutex> lk(mutex_);
    AppendUnlocked(log_line, len);
  } else
    AppendUnlocked(log_line, len);
}
void LogFile::Flush() {
  if (thread_safe_) {
    std::lock_guard<std::mutex> lk(mutex_);
    file_->Flush();
  } else
    file_->Flush();
}
// 二、日志文件冲刷的情况：
// 1. 文件分割（析构函数中调用fclose，自动冲刷）
// 2. 缓冲区（setbuffer设置的缓冲区）满了
// 3. 当前时间距离上次冲刷时间（last_flush_）达到了冲刷间隔（flush_interval_）
// 4. Flush()被调用
void LogFile::AppendUnlocked(const char* logline, int len) {
  file_->Append(logline, len);
  if (file_->WrittenBytes() > roll_size_)
    RollFile();
  else {
    // TODO: 调用了n次time调用？
    ++count_;
    if (count_ >= check_every_n_)  //保证一个日志文件最少有check_every_n条记录
    {
      count_ = 0;
      time_t now = ::time(nullptr);
      time_t this_period = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (this_period != start_of_period_)
        if (RollFile()) {
        } else if (now - last_flush_ > flush_interval_) {
          last_flush_ = now;
          file_->Flush();
        }
    }
  }
}
// Todo(leiyx): 这里可以改进一下
std::string LogFile::GetLogFileName(const std::string& basename, time_t* now) {
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  gmtime_r(now, &tm);
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", getpid());
  filename += pidbuf;

  filename += ".log";
  return filename;
}
// 一、日志文件分割的情况：
// 1. 新的一天
// 2. 内容达到roll_size_字节
bool LogFile::RollFile() {
  time_t now = 0;
  std::string file_name = GetLogFileName(base_name_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;
  if (now > last_roll_) {
    last_roll_ = now;
    last_flush_ = now;
    start_of_period_ = start;
    file_.reset(new AppendFile(file_name));
    // reset做两件事：
    // 1.销毁当前指针所指对象，调用AppendFile类的析构函数，则文件指针fp_被关闭（RAII手法）
    // 2.并使该智能指针指向新的对象,调用AppendFile类的构造函数，新文件被建立，written_bytes_重置为0
    return true;
  }
  return false;
}