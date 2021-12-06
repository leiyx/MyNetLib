#include "logger.h"

#include "callbacks.h"
#include "current_thread.h"

extern const int kBUfferSize;

void DefaultOutput(const char* msg, int len) {
  size_t n = fwrite(msg, 1, len, stdout);
  assert(n == len);
}
void DefaultFlush() { fflush(stdout); }

// 默认将日志信息输出和冲刷到标准输出（终端）
OutputFunc g_output = DefaultOutput;
FlushFunc g_flush = DefaultFlush;
void Logger::SetOutputFunc(OutputFunc out) { g_output = out; }
void Logger::SetFlushFunc(FlushFunc flush) { g_flush = flush; }

// 初始化日志级别，可在最外层CMakeLists.txt文件中定义不同的宏来启用不同的日志等级
Logger::LogLevel initLogLevel() {
#ifdef USE_LOG_TRACE
  return Logger::TRACE;
#elif defined USE_LOG_DEBUG
  return Logger::DEBUG;
#else
  return Logger::INFO;
#endif
}
Logger::LogLevel g_logLevel = initLogLevel();

inline Logger::LogLevel Logger::GetLogLevel() { return g_logLevel; }
void Logger::SetLogLevel(Logger::LogLevel level) { g_logLevel = level; }
const char* LogLevelName[Logger::FATAL + 1] = {
    "[TRACE]: ", "[DEBUG]: ", "[INFO]:  ",
    "[WARN]:  ", "[ERROR]: ", "[FATAL]: ",
};

// 日志打印流程：Logger临时对象构造-> impl_对象构造（组装日志信息）->
// Logger临时对象析构 -> impl_.Finish()添加文件名、行号信息 ->
// Stream().GetBuffer()将该条日志消息取出存放在buf中 -> 调用设置的output ->
// 调用设置的flush;
Logger::~Logger() {
  impl_.Finish();
  const FixedBuffer<kSmallBufferSize>& buf(Stream().GetBuffer());
  g_output(buf.GetData(), buf.Length());
  if (impl_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

// 编译期计算字符串data的长度
class T {
 public:
  T(const char* data, unsigned len) : data_(data), len_(len) {
    assert(strlen(data) == len_);
  }
  const char* data_;
  const unsigned len_;
};
inline LogStream& operator<<(LogStream& s, T v) {
  s.Append(v.data_, v.len_);
  return s;
}

// impl_对象的构造，将消息组装完毕存入Stream的缓冲区
Logger::Impl::Impl(Logger::LogLevel level, const char* filename, int line)
    : stream_(), level_(level), line_(line), filename_(filename) {
  const char* temp = CurrentThread::TidAsString().c_str();
  stream_ << T(temp, strlen(temp)) << " " << TimeStamp::Now().TimeToString()
          << " " << T(LogLevelName[level], 9) << " ";
}
void Logger::Impl::Finish() {
  stream_ << " - " << filename_ << ':' << line_ << "\n";
}
