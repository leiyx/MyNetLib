#include "logger.h"
#include "utils.h"
// 同步日志打印：输出到标准输出（控制台）
void Log1() {
  for (int i = 0; i < 10000; i++) {
    LOG_INFO << "同步日志打印：输出到标准输出（控制台）";
  }
}

// 同步日志打印：输出到文件
std::unique_ptr<LogFile> g_logfile;
void MyOutput(const char *msg, int len) { g_logfile->Append(msg, len); }
void MyFlush() { g_logfile->Flush(); }

void Log2() {
  g_logfile.reset(new LogFile("test_sync_logfile.log", 200 * 1000 * 1000));
  Logger::SetOutputFunc(MyOutput);
  Logger::SetFlushFunc(MyFlush);
  for (int i = 0; i < 10000; i++) {
    LOG_INFO << "同步日志打印：输出到文件";
  }
}
int main() {
  // utils::TestTime(Log1);
  utils::TestTime(Log2);
  return 0;
}
// TODO:日志信息中文和行号乱码问题