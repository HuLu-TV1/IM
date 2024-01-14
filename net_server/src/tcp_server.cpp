#include "tcp_server.h"
#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "socket.h"
#include "tcp_connection.h"
#include "thread_pool.h"
#include <acceptor.h>
#include <cstring>
#include <functional>
#include <unistd.h>
#include "log.h"

#include <stdio.h>
using namespace net;
Server::Server(EventLoop *eventloop, InetAddress &addr, int threads_num)
    : main_eventloop_(eventloop), thread_num_(threads_num),
      message_callback_(DefaultMessageCb),
      connection_callback_(DefaultConnectCb)
{
  Acceptor *accptor = new Acceptor(main_eventloop_, addr);
  std::function<void(int)> connectCb =
      std::bind(&Server::NewConnect, this, std::placeholders::_1);
  accptor->set_new_connection_callback(connectCb);
  threads_pools_ = std::make_unique<ThreadPool>(thread_num_);
  threads_pools_->Start();
  for (int i = 0; i < thread_num_; i++)
  {
    
      char tmp[25];
      sprintf(tmp, "%d", i); // 使用 sprintf() 进行格式化输入
    std::string name = "loop";
    name +=tmp;
    std::unique_ptr<EventLoop> sub_loop = std::make_unique<EventLoop>(name);
    sub_eventloops_.emplace_back(std::move(sub_loop));
  }
}
void Server::Start()
{
  for (int i = 0; i < static_cast<int>(sub_eventloops_.size()); i++)
  {
    std::function<void()> sub_loop =
        std::bind(&EventLoop::Loop, sub_eventloops_[i].get());
    threads_pools_->Run(sub_loop);
  }
  main_eventloop_->Loop();
}

void Server::NewConnect(int client_fd)
{
  static int next = 0;
  EventLoop *sub_eventloop = sub_eventloops_[next].get();
  TcpConnectionPtr connection =
      std::make_shared<Connection>(sub_eventloop, client_fd);
  // std::unique_ptr<Connection> connection =
  //     std::make_unique<Connection>(main_eventloop_, client_fd);
  connection->set_message_callback(message_callback_);
  connection->set_connect_callback(connection_callback_);
  next++;
  if (next >= static_cast<int>(sub_eventloops_.size()))
  {
    next = 0;
  }
  std::function<void(const TcpConnectionPtr&)> deleteCb =
      std::bind(&Server::DeleteConnection, this, std::placeholders::_1);
  connection->set_delete_connection_callback(deleteCb);
  //connections_[client_fd] = std::move(connection);
  connections_[client_fd] = connection;
  sub_eventloop->RunInLoop(std::bind(&Connection::ConnectEstablished,connection));
   LOG_DEBUG("Server::NewConnect  = %d",client_fd);
}

void Server::DeleteConnection(const TcpConnectionPtr&conn)
{
  main_eventloop_->RunInLoop(std::bind(&Server::RemoveConnectionInLoop,this,conn));
}

void Server::RemoveConnectionInLoop(const TcpConnectionPtr&conn) {
  main_eventloop_->assertInLoopThread();
  LOG_DEBUG("Server::rRemoveConnectionInLoope  = %d",conn->get_socket_fd());
  size_t n = connections_.erase(conn->get_socket_fd());
   (void)n;
  EventLoop*sub_eventloop = conn->get_loop();
  sub_eventloop->RunInLoop(std::bind(&Connection::ConnectDestroyed,conn));
}
void Server::DefaultMessageCb(const TcpConnectionPtr&, Buffer *buf)
{
  buf->RetrieveAll();
}
void Server::DefaultConnectCb(const TcpConnectionPtr&)
{
  // nothing
}
