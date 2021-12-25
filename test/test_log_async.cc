#include "async_logging.h"
#include "logger.h"
#include "utils.h"
// 同步日志打印：输出到文件
AsyncLogging* g_async_logfile = nullptr;
void MyOutput(const char* msg, int len) { g_async_logfile->Append(msg, len); }
void MyFlush() {}

void Log() {
  AsyncLogging log("test_async_logfile.log", 400 * 1000 * 1000);
  g_async_logfile = &log;
  Logger::SetOutputFunc(MyOutput);
  Logger::SetFlushFunc(MyFlush);
  Logger::SetLogLevel(Logger::LogLevel::DEBUG);
  log.Start();
  for (int i = 0; i < 100000; i++) {
    LOG_INFO << "异步日志打印：输出到文件";
  }
}
int main() {
  utils::TestTime(Log);
  return 0;
}