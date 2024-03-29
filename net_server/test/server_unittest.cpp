#include "event_loop.h"
#include "inet_address.h"
#include "tcp_server.h"
using namespace net;
int main() {
  InetAddress serv_addr(8888, "127.0.0.1");
  EventLoop eventloop("main");
  Server server(&eventloop, serv_addr,10);
  server.Start();
  return 0;
}