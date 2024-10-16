#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;
void resethandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc, char **argv)
{
    signal(SIGINT, resethandler);
    EventLoop loop;
    if (argc < 3)
    {
        cerr << "command invalid example : ./ChatService 127.0.0.1 6000" << endl;
        exit(-1);
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, " Chatserver");
    server.start();
    loop.loop();
    return 0;
}
