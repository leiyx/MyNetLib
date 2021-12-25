#include "tcp_server.h"

#include <strings.h>

#include "socket_utils.h"

EventLoop* CheckLoopNotNull(EventLoop* loop) {
  if (loop == nullptr) LOG_FATAL << "mainloop is null!";
  return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddr& listen_addr,
                     const std::string& name_arg, Option option)
    : loop_(CheckLoopNotNull(loop)),
      name_(name_arg),
      ip_port_(listen_addr.ToIpPort()),
      acceptor_(new Acceptor(loop, listen_addr, option == kReusePort)),
      next_conn_id_(1),
      connection_maps(),
      thread_pool_(new EventLoopThreadPool(loop, name_)),
      started_(0)  // 没有初始化，线程池启动不了
{
  acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2));
}
TcpServer::~TcpServer() {
  for (auto& item : connection_maps) {
    TcpConnectionPtr conn(item.second);
    // 无需从connection_maps中移除了，因为马上TcpServer要析构了，不再使用该map了
    item.second.reset();
    conn->GetLoop()->RunInLoop(
        std::bind(&TcpConnection::ConnectionDestroyed, conn));
  }
}

void TcpServer::SetThreadNum(int num) { thread_pool_->SetThreadNum(num); }

void TcpServer::Start() {
  if (started_++ == 0)  // 防止一个TcpServer对象被start多次
  {
    thread_pool_->Start(thread_init_callback_);
    loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
  }
  LOG_DEBUG << "Server started!";
}

void TcpServer::NewConnection(int sockfd, const InetAddr& peer_addr) {
  EventLoop* io_loop = thread_pool_->GetNextLoop();
  char buf[64] = {0};
  snprintf(buf, sizeof buf, "-%s#%d", ip_port_.c_str(), next_conn_id_++);
  std::string conn_name = name_ + buf;
  LOG_INFO << "TcpServer::newConnection [" << name_.c_str()
           << "] - new connection [" << conn_name.c_str() << "%s] from "
           << peer_addr.ToIpPort().c_str();
  InetAddr local_addr(socket_utils::GetLocalAddr(sockfd));

  TcpConnectionPtr conn(
      new TcpConnection(io_loop, local_addr, peer_addr, sockfd, conn_name));
  connection_maps[conn_name] = conn;

  conn->SetConnectionCallback(connection_callback_);
  conn->SetMessageCallback(message_callback_);
  conn->SetWriteCompleteCallback(write_complete_callback_);

  conn->SetCloseCallback(
      std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));

  io_loop->RunInLoop(std::bind(&TcpConnection::ConnectionEstablished, conn));
}
void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->RunInLoop(std::bind(&TcpServer::RemoveconnectionInLoop, this, conn));
}
void TcpServer::RemoveconnectionInLoop(const TcpConnectionPtr& conn) {
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_.c_str()
           << "] - connection " << conn->Name().c_str();

  connection_maps.erase(conn->Name());
  EventLoop* io_loop = conn->GetLoop();
  io_loop->QueueInLoop(std::bind(&TcpConnection::ConnectionDestroyed, conn));
}