#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace std;
using namespace muduo;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MESSAGE, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREAT_GROUP_MSG, std::bind(&ChatService::creatGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    if (_redis.connect())
    {
        _redis.init_notify_handler(bind(&ChatService::handlerRedisSubscribeMessage, this, _1, _2));
    }
}

// 服务器异常，业务重置方法
void ChatService::reset()
{
    // 把online状态的用户，设置成offline
    _usermodel.resetState();
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}
void ChatService::handlerRedisSubscribeMessage(int userid,string msg)
{
    json js=json::parse(msg.c_str());
    lock_guard<mutex> lock(_connMutex);
    auto it=_userConnMap.find(userid);
    if(it!=_userConnMap.end())
    {
        it->second->send(msg);
        return;
    }
    _offlinemsagemodel.insert(userid,msg);
}

// 处理登录业务  id  pwd   pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _usermodel.query(id);
    if (user.getID() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }
            _redis.subscribe(id);
            // 登录成功，更新用户状态信息 state offline=>online
            user.setState("online");
            _usermodel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getID();
            response["name"] = user.getName();
            // 查询该用户是否有离线消息
            vector<string> vec = _offlinemsagemodel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlinemsagemodel.remove(id);
            }

            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendmodel.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getID();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            vector<Group> groupVec = _groupmodel.query(id);
            if (!groupVec.empty())
            {
                vector<string> vec2;
                for (Group &user : groupVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["groupname"] = user.getName();
                    js["groupsec"] = user.getDesc();
                    vector<string> a;
                    for (auto &ch : user.getUser())
                    {
                        json j;
                        j["id"] = ch.getID();
                        j["name"] = ch.getName();
                        j["role"] = ch.getRole();
                        j["state"] = ch.getState();
                        a.push_back(j.dump());
                    }
                    js["users"] = a;
                    vec2.push_back(js.dump());
                }
                response["groups"] = vec2;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 该用户不存在，用户存在但是密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password is invalid!";
        conn->send(response.dump());
    }
}

// 处理注册业务  name  password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _usermodel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getID();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(js["id"]);
        if (it != _userConnMap.end())
        {
            if (it->second == conn)
            {
                // 从map表删除用户的链接信息
                user.setID(it->first);
                _userConnMap.erase(it);
            }
            user.setID(js["id"]);
            user.setState("offline");
            _usermodel.updateState(user);
        }
    }
    _redis.unsubscribe(js["id"]);
}
// 处理客户端异常退出
void ChatService::clientCloseExpection(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的链接信息
                _redis.unsubscribe(it->first);
                user.setID(it->first);
                _userConnMap.erase(it);
                break;
            }
            if (user.getID() != -1)
            {
                user.setState("offline");
                _usermodel.updateState(user);
            }
        }
    }
    // 更新用户的状态信息
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid在线，转发消息   服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }
    User user = _usermodel.query(js["toid"]);
    if (user.getState() != "offline")
    {
        _redis.publish(js["toid"], js.dump());
        return;
    }
    // toid不在线，存储离线消息
    _offlinemsagemodel.insert(toid, js.dump());
}

// 添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息
    _friendmodel.insert(userid, friendid);
}
void ChatService::creatGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    Group group(js["id"], js["groupname"], js["groupdesc"]);
    if (_groupmodel.creatGroup(group))
    {
        _groupmodel.addGroup(js["id"], group.getId(), "creator");
    }
}
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    _groupmodel.addGroup(js["id"], js["groupid"], "normal");
}
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"];
    int groupid = js["groupid"];
    vector<int> result = _groupmodel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for (auto &n : result)
    {

        auto it = _userConnMap.find(n);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            User user = _usermodel.query(n);
            if (user.getState() != "offline")
            {
                _redis.publish(n, js.dump());
                return;
            }
            _offlinemsagemodel.insert(n, js.dump());
        }
    }
}
