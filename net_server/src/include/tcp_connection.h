#ifndef __CONNECTION__
#define __CONNECTION__
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "buffer.h"
#include "callback.h"
#include "noncopyable.h"

namespace net
{
  class EventLoop;
  class Socket;
  class Channel;
  class Connection : noncopyable, public std::enable_shared_from_this<Connection>
  {
  public:
    Connection(EventLoop *eventloop, int client_fd);
    ~Connection();
    void set_delete_connection_callback(const DeleteConnectCallback &cb)
    {
      deleteConnectionCallback_ = cb;
    }
    void set_connect_callback(const ConnectCallback &cb)
    {
      connectio_callback_ = cb;
    }

    void set_message_callback(const OnMessageCallback &cb)
    {
      message_callback_ = std::move(cb);
    }
    void send(std::string str);
    void ShutDownWrite();
    void shutdown();
    void ConnectEstablished();
    void ConnectDestroyed();

    size_t get_socket_fd() const {return client_fd_;}
    EventLoop *get_loop() const {return eventloop_;}

  private:
    enum StateE
    {
      KDisconnected,
      Kconnecting,
      KConnected,
      KDisconnecting
    };
    void setState(StateE s) { state_ = s; }

    void HandleRead();
    void HandleWrite();
    void HandleClose();
    void HandleError();
    ConnectCallback connectio_callback_;
    OnMessageCallback message_callback_;
    DeleteConnectCallback deleteConnectionCallback_;
    EventLoop *eventloop_;
    std::unique_ptr<Socket> clntSock_;
    std::unique_ptr<Channel> client_channel_;
    Buffer input_buf_;
    Buffer output_buf_;
    StateE state_;
    int client_fd_;
  };
} // namespace net
#endif