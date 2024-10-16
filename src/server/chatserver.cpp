#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <string>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册连接回调
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));
    // 注册消息回调
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
    // 设置线程数
    _server.setThreadNum(4);
}
void ChatServer::start()
{
    _server.start();
}
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{ // 用户断开连接
    if (!conn->connected())
    {
        ChatService ::instance()->clientCloseExpection(conn);
        conn->shutdown();
    }
}
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    json js = json::parse(buf);
    // 通过js["megid"]
    auto msgHandler = ChatService ::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, time);
}