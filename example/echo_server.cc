/*
 * @Author: lei
 * @Description: 在回射服务器版本1的基础上
 * 注册定时事件,且实现异步打印日志到本地文件
 * @FilePath: /MyNetLib/example/echo_server.cc
 */

#include <unistd.h>

#include <functional>
#include <string>

#include "config.h"
#include "tcp_server.h"
void func(std::string& name) { std::cout << name << "timer on!" << std::endl; }

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddr& addr, const std::string& name,
             int thread_num)
      : server_(loop, addr, name), loop_(loop) {
    server_.SetConnectionCallback(
        std::bind(&EchoServer::OnConnection, this, std::placeholders::_1));
    server_.SetMessageCallback(
        std::bind(&EchoServer::OnMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
    server_.SetThreadInitCallback(
        std::bind(&EchoServer::OnThreadInit, this, std::placeholders::_1));
    server_.SetWriteCompleteCallback(
        std::bind(&EchoServer::OnWriteComplete, this, std::placeholders::_1));
    server_.SetThreadNum(thread_num);
  }

  void Start() { server_.Start(); }

 private:
  EventLoop* loop_;
  TcpServer server_;

  //连接建立和断开
  void OnConnection(const TcpConnectionPtr& conn) {
    if (conn->Connected()) {
      LOG_INFO << "conn UP: " << conn->PeerAddr().ToIpPort().c_str();
    } else
      LOG_INFO << "conn DOWN: " << conn->PeerAddr().ToIpPort().c_str();
  }
  //读事件回调
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, TimeStamp time) {
    std::string msg = buf->RetriveAllAsString();
    // Process(),业务逻辑代码；如果是回射服务器，那么这里就不用了，可以直接将收到数据发送给对应客户端
    conn->Send(msg);
    // conn->ShutDown();
  }
  //线程初始化回调
  void OnThreadInit(EventLoop* loop) {
    LOG_INFO << "thread " << CurrentThread::Tid() << " is created!";
  }
  //写事件完成回调
  void OnWriteComplete(const TcpConnectionPtr& conn) {
    LOG_INFO << "来自 " << conn->PeerAddr().ToIpPort().c_str()
             << " 的客户端请求已处理完毕！";
  }
};

AsyncLogging* g_async_logfile = nullptr;
void MyOutput(const char* msg, int len) { g_async_logfile->Append(msg, len); }

int main() {
  AsyncLogging async_logfile("testlog", 300 * 1000 * 1000, 1);
  async_logfile.Start();
  g_async_logfile = &async_logfile;
  Logger::SetOutputFunc(MyOutput);

  Config config;
  config.PullConfig("server.conf");

  EventLoop loop;
  InetAddr addr(atoi(config.mp[std::string("server_listen_port")].c_str()));
  EchoServer echo_server(
      &loop, addr, config.mp[std::string("server_name")],
      atoi(config.mp[std::string("server_thread_num")].c_str()));
  echo_server.Start();
  loop.Loop();
  return 0;
}
