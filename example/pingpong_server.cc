/*
 * @Author: lei
 * @Description: pingpong 测试的echo_server
 * @FilePath: /MyNetLib/example/pingpong_server.cc
 */

#include <unistd.h>

#include <functional>
#include <string>

#include "config.h"
#include "logger.h"
#include "tcp_server.h"

class PingpongServer {
 public:
  PingpongServer(EventLoop* loop, const InetAddr& addr, const std::string& name,
                 int thread_num)
      : server_(loop, addr, name), loop_(loop) {
    server_.SetConnectionCallback(
        std::bind(&PingpongServer::OnConnection, this, std::placeholders::_1));
    server_.SetMessageCallback(
        std::bind(&PingpongServer::OnMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
    server_.SetThreadNum(thread_num);
  }

  void Start() { server_.Start(); }

 private:
  EventLoop* loop_;
  TcpServer server_;

  // 连接建立和断开
  void OnConnection(const TcpConnectionPtr& conn) { conn->SetTcpNoDelay(true); }
  // 读事件回调
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, TimeStamp time) {
    // conn->Send(buf);
    std::string buffer = buf->RetriveAllAsString();
    conn->Send(buffer);
  }
};

AsyncLogging* g_async_logfile = nullptr;
void MyOutput(const char* msg, int len) { g_async_logfile->Append(msg, len); }

int main(int argc, char** argv) {
  AsyncLogging async_logfile("testlog", 400 * 1000 * 1000, 1);
  async_logfile.Start();
  g_async_logfile = &async_logfile;
  Logger::SetOutputFunc(MyOutput);

  if (argc < 4) {
    // 可执行文件 地址 端口 线程数
    fprintf(stderr, "Usage: server <ip_address> <port> <threads>\n");
    exit(-1);
  } else {
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::Tid();
    Logger::SetLogLevel(Logger::ERROR);
  }
  const char* ip = argv[1];
  uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
  InetAddr listen_addr(ip, port);
  int thread_count = atoi(argv[3]);

  EventLoop loop;

  PingpongServer server(&loop, listen_addr, "PingPong", thread_count);

  server.Start();

  loop.Loop();

  return 0;
}

// ./pingpong_server 0.0.0.0 55555 1