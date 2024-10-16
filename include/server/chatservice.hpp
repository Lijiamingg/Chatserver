#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <unordered_map>
#include <muduo/net/TcpServer.h>
#include <functional>
#include <mutex>
#include "json.hpp"
#include "UserModel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
using namespace muduo;
using namespace muduo::net;
using namespace std;
using json = nlohmann::json;
using MsgHandler = function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;
// 聊天服务器业务类
class ChatService
{
private:
    mutex _connMutex;
    unordered_map<int,MsgHandler> _msgHandlerMap;
    unordered_map<int,TcpConnectionPtr> _userConnMap;
    unordered_map<TcpConnectionPtr,int> _re_userConnMap;
    ChatService();
    UserModel _usermodel;
    Groupmodel _groupmodel;
    Friendmodel _friendmodel;
    Redis _redis;
    offlinemessagemodel _offlinemsagemodel;
public:
    //获取单例对象的接口函数
    void reset();
    static ChatService* instance();
    //处理登录和注册业务
    void clientCloseExpection(const TcpConnectionPtr &conn);
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void creatGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void handlerRedisSubscribeMessage(int userid,string msg);
    MsgHandler getHandler(int msgid);
};

#endif