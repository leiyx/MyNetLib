#include "tcp_connection.h"

#include "channel.h"
#include "socket.h"
#include "socket_utils.h"
#include "timer.h"

static EventLoop* CheckLoopNotNull(EventLoop* loop) {
  if (loop == nullptr) LOG_FATAL << " TcpConnection Loop is null!";
  return loop;
}

TcpConnection::TcpConnection(EventLoop* loop, const InetAddr& local_addr,
                             const InetAddr& peer_addr, int fd,
                             const std::string& name)
    : loop_(CheckLoopNotNull(loop)),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      socket_(new Socket(fd)),
      channel_(new Channel(loop_, fd)),
      name_(name),
      state_(kConnecting) {
  channel_->SetReadEventCallback(
      std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));
  channel_->SetWriteEventCallback(std::bind(&TcpConnection::HandleWrite, this));
  channel_->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
  channel_->SetConnectionCloseCallback(
      std::bind(&TcpConnection::HandleClose, this));
  LOG_INFO << "TcpConnection::ctor[" << name_.c_str() << "] at fd=" << fd;
  socket_utils::SetKeepAlive(fd, true);
}

TcpConnection::~TcpConnection() {
  LOG_INFO << "TcpConnection::dtor[" << name_.c_str()
           << "] at fd=" << channel_->Fd() << "state=" << (int)state_;
}

void TcpConnection::Send(const std::string& buf) {
  if (state_ == kConnected) {
    if (loop_->IsInLoopThread()) {
      SendInLoop(buf.c_str(), buf.size());
    } else {
      loop_->RunInLoop(
          std::bind(&TcpConnection::SendInLoop, this, buf.c_str(), buf.size()));
    }
  }
}
void TcpConnection::Send(const char* buf, int len) {
  if (state_ == kConnected) {
    if (loop_->IsInLoopThread()) {
      SendInLoop(buf, len);
    } else {
      loop_->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, buf, len));
    }
  }
}
void TcpConnection::SendInLoop(const void* data, size_t len) {
  ssize_t nwrote = 0;
  ssize_t remaining = len;
  bool faultError = false;

  if (state_ == kDisconnected) {
    LOG_ERROR << "disconnected,give up writing!";
    return;
  }
  // 如果channel是第一次发数据
  if (!channel_->IsWriting() && output_buffer_.ReadableBytes() == 0) {
    nwrote = ::write(channel_->Fd(), data, len);
    if (nwrote > 0) {
      remaining = len - nwrote;
      if (remaining == 0 && write_complete_callback_) {
        loop_->RunInLoop(
            // 为什么shared_from_this?
            std::bind(write_complete_callback_, shared_from_this()));
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_ERROR << "TcpConnection::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) {
          faultError = true;
        }
      }
    }
  }

  if (!faultError && remaining > 0) {
    // 高水位做的事情
    output_buffer_.Append((char*)data + nwrote, remaining);
    if (!channel_->IsWriting()) channel_->EnableWrite();
  }
}

void TcpConnection::ShutDown() {
  if (state_ == kConnected) {
    SetState(kDisconnecting);
    loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
  }
}
void TcpConnection::ShutdownInLoop() {
  while (channel_->IsWriting())  // 等待该channel要发的数据发完，再关闭
    ;
  socket_->ShutdownWrite();
}

void TcpConnection::ConnectionEstablished() {
  SetState(kConnected);
  channel_->Tie(shared_from_this());
  channel_->EnableRead();
  connection_callback_(shared_from_this());
}
void TcpConnection::ConnectionDestroyed() {
  if (state_ == kConnected)  // 一般情况下不会进来
  {
    SetState(kDisconnected);
    channel_->DisableAll();
    connection_callback_(shared_from_this());
  }
  channel_->Remove();  // 把channel从poller中删除掉
}

void TcpConnection::HandleRead(TimeStamp receive_time) {
  int save_errno = 0;
  //将从到达数据从socket接收缓冲区 读到 input_buffer_缓冲区（应用程序缓冲区）
  ssize_t n = input_buffer_.ReadFd(channel_->Fd(), &save_errno);

  if (n > 0) {
    message_callback_(shared_from_this(), &input_buffer_, receive_time);
  } else if (n == 0) {
    HandleClose();
  } else {
    errno = save_errno;
    LOG_ERROR << "error:" << errno << "occured!";
    HandleError();
  }
}
// TODO:这里采用的LT模式，未采用ET模式
void TcpConnection::HandleWrite() {
  if (channel_->IsWriting()) {
    int save_errno = 0;
    //将待发送数据从output_buffer_用户缓冲区 写到 socket发送缓冲区
    ssize_t n = output_buffer_.WriteFd(channel_->Fd(), &save_errno);
    if (n > 0) {
      output_buffer_.Retrive(n);
      if (output_buffer_.ReadableBytes() == 0) {
        channel_->DisableWrite();
        if (write_complete_callback_)
          loop_->RunInLoop(
              std::bind(write_complete_callback_, shared_from_this()));
        if (state_ == kDisconnecting) ShutdownInLoop();
      }
    } else {
      LOG_ERROR << "TcpConnection::HandleWrite";
    }
  } else {
    LOG_ERROR << "TcpConnection fd=<<" << channel_->Fd()
              << " is down, no more writing";
  }
}
void TcpConnection::HandleClose() {
  LOG_INFO << "TcpConnection::handleClose fd=" << channel_->Fd()
           << " ,state=" << static_cast<int>(state_);
  SetState(kDisconnected);
  channel_->DisableAll();

  TcpConnectionPtr connPtr(shared_from_this());
  connection_callback_(connPtr);
  close_callback_(connPtr);
}
void TcpConnection::HandleError() {
  int optval;
  socklen_t optlen = sizeof optval;
  int err = 0;
  if (::getsockopt(channel_->Fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) <
      0) {
    err = errno;
  } else {
    err = optval;
  }
  LOG_ERROR << "TcpConnection::handleError name:" << name_.c_str()
            << " - SO_ERROR:" << err;
}
