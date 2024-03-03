#include "tcp_connection.h"
#include "channel.h"
#include "log.h"
#include "socket.h"
#include "sockets_ops.h"
#include <chrono>
#include <string.h>
#include <sys/prctl.h>
#include <thread>
#include <unistd.h>
#include "event_loop.h"
using namespace net;
void Print()
{
  char thread_name[256];
  pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name));
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  LOG_DEBUG("thread_name = %s", thread_name);
  LOG_DEBUG("std::this_thread::get_id is  = %d", std::this_thread::get_id());
}
Connection::Connection(EventLoop *eventloop, int client_fd)
    : eventloop_(eventloop), clntSock_(std::make_unique<Socket>(client_fd)), state_(Kconnecting), client_fd_(client_fd)
{
  int connect_fd_ = clntSock_->get_socket_fd();
  clntSock_->set_nonblocking(client_fd);
  LOG_DEBUG(" eventloop_ name = %s connect_fd = %d", eventloop_->name_.c_str(), connect_fd_);
  if (eventloop != nullptr)
  {
    client_channel_ = std::make_unique<Channel>(eventloop_, connect_fd_);
    client_channel_->set_handle_read_event(
        std::bind(&Connection::HandleRead, this));
    client_channel_->set_handle_write_event(
        std::bind(&Connection::HandleWrite, this));
    client_channel_->set_handle_error_event(
        std::bind(&Connection::HandleError, this));
    client_channel_->set_handle_close_event(
        std::bind(&Connection::HandleClose, this));
    // client_channel_->EnableRead();
  }
}

Connection::~Connection() { assert(state_ == KDisconnected); }

void Connection::HandleError()
{
  int err = get_socket_error(client_channel_->get_fd());
  LOG_DEBUG("TcpConnectionPtr::HandleError [] - SO_ERROR =%d err=%s", err,
            strerror(err));
}

void Connection::HandleRead()
{
  int savedErrno;
  ssize_t len = input_buf_.ReadFd(client_channel_->get_fd(), &savedErrno);
  LOG_DEBUG("HandleRead len = %d", len);
  if (len > 0)
  {
    message_callback_(shared_from_this(), &input_buf_);
  }
  else if (len == 0)
  {
    LOG_DEBUG("len == 0");
    HandleClose();
  }
  else
  {
    errno = savedErrno;
    HandleError();
  }
}
void Connection::HandleWrite()
{

  if (client_channel_->IsWriting())
  {
     LOG_DEBUG("HandleWrite");
    ssize_t n = write(clntSock_->get_socket_fd(), output_buf_.Peek(), output_buf_.ReadableBytes());
    if (n > 0)
    {
      output_buf_.Retrieve(n);
      if (output_buf_.ReadableBytes() == 0)
      {
        client_channel_->DisableWrite();
        if (state_ == KDisconnected)
        {
          clntSock_->shutdownWrite();
        }
      }
      else
      {
        LOG_DEBUG("TcpConnectionPtr::handleWrite");
      }
    }
  }
  else
  {
    LOG_DEBUG("Connection fd =  = %d,is down, no more writing", client_channel_->get_fd());
  }
}
void Connection::HandleClose()
{
  ShowCurrentState();
  LOG_DEBUG("close fd = %d state= %s", client_channel_->get_fd(), StateStr_.c_str());
  assert(state_ == KConnected || state_ == KDisconnecting);
  setState(KDisconnected);
  connectio_callback_(shared_from_this());
  client_channel_->DisableAll();
  deleteConnectionCallback_(shared_from_this());
}

void Connection::ShowCurrentState()
{
  switch (state_)
  {
  case KDisconnected:
    StateStr_ = "KDisconnected";
    break;
  case Kconnecting:
    StateStr_ = "Kconnecting";
    break;
  case KConnected:
    StateStr_ = "KConnected";
    break;
  case KDisconnecting:
    StateStr_ = "KDisconnecting";
    break;

  default:
    break;
  }
}
void Connection::send(std::string str)
{
  if (state_ == KDisconnected)
  {
    LOG_DEBUG("disconnected,give up writing");
    return;
  }
  ssize_t nwrote = 0;
  size_t remaining = str.size();
  size_t len = str.size();
  bool faultError = false;
  if (!client_channel_->IsWriting() && output_buf_.ReadableBytes() == 0)
  {
      LOG_DEBUG("size = %d",len);
    nwrote = write(clntSock_->get_socket_fd(), str.c_str(), len);
    if (nwrote >= 0)
    {
      remaining = len - nwrote;
      if (remaining == 0)
      {
        char thread_name[255];
        pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name));
        LOG_DEBUG("thread_name =%s send message complete", thread_name);
      }
    }
    else
    {
      nwrote = 0;
      if (errno != EWOULDBLOCK)
      {
        if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
        {
          faultError = true;
        }
      }
    }
  }

  if (!faultError && remaining > 0)
  {
    // std::string tmp(str.c_str()+nwrote);
    output_buf_.Append(str.c_str() + nwrote, remaining);
    if (!client_channel_->IsWriting())
    {
      client_channel_->EnableWrite();
    }
  }
}

void Connection::ShutDownWrite()
{
  if (!client_channel_->IsWriting())
  {
    clntSock_->shutdownWrite();
  }
}

void Connection::shutdown()
{
  if (state_ == KConnected)
  {
    setState(KDisconnecting);
    eventloop_->RunInLoop(std::bind(&Connection::shutdownInLoop, this));
  }
}

void Connection::ConnectEstablished()
{
  LOG_DEBUG("ConnectEstablished");
  eventloop_->assertInLoopThread();
  LOG_DEBUG("state_ = %d", state_);
  assert(state_ == Kconnecting);
  setState(KConnected);
  // client_channel_->tie();
  client_channel_->EnableRead();
  LOG_DEBUG("ConnectEstablished");
}
void Connection::ConnectDestroyed()
{
  eventloop_->assertInLoopThread();
   LOG_DEBUG("ConnectDestroyed");
  if (state_ == KConnected)
  {
    setState(KDisconnected);
    client_channel_->DisableAll();
    connectio_callback_(shared_from_this());
  }
}

void Connection::shutdownInLoop()
{
  eventloop_->assertInLoopThread();
  LOG_DEBUG("shutdownInLoop");
  if (!client_channel_->IsWriting())
  {
    // we are not writing
    clntSock_->shutdownWrite();
  }
}