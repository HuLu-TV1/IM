#include "event_loop.h"
#include "inet_address.h"
#include "tcp_server.h"
#include <string>
#include<iostream>
#include "log.h"
#include <memory>
using namespace net;
class EchoServer{
    public:
    EchoServer(EventLoop *eventloop, InetAddress&serv_addr) :event_loop_(eventloop),server_(eventloop,serv_addr,5) {
    server_.set_connect_cb(std::bind(&EchoServer::onConnect,this,_1));
    server_.set_message_cb(std::bind(&EchoServer::OnMessage,this,_1,_2));
    }
    void Start() {
        server_.Start();
    }
    private:
        void OnMessage( const std::shared_ptr<Connection>&connc, Buffer *readbuf) {
        std::string str(readbuf->RetrieveAllAsString());
        LOG_DEBUG("receive str = %s",str.c_str());
        connc->send(str);
        connc->shutdown();
    }
    void onConnect( const std::shared_ptr<Connection>&connc) {
        connc->send("bye");
    }
    private:
    EventLoop *event_loop_;
    Server server_;
};
int main() {
 CommonLog::Logger::GetInstance()->Init(CommonLog::LogType::LOG_PRINT, 0);
  InetAddress serv_addr(8888, "127.0.0.1");
  EventLoop event_loop("main");
  EchoServer echo_server(&event_loop, serv_addr);
  echo_server.Start();
  return 0; 
}