#include "json.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <ctime>
#include <chrono>
using namespace std;
using json = nlohmann::json;
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

User g_currentUser;
vector<User> g_currentUserFriendList;
vector<Group> g_currentUserGroupList;
void showCurrentUserData();
void readTaskHandler(int clientid);
string getCurrentTime();
void mainMenu(int clientid);
void help(int clientid = 0, string s = "");
void chat(int clientid, string s);
void addfriend(int clientid, string s);
void creategroup(int clientid, string s);
void addgroup(int clientid, string s);
void groupchat(int clientid, string s);
void loginout(int clientid, string s);
bool work=true;
// 主线程发送线程，子线程接受线程
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid example : ./ChatService 127.0.0.1 6000" << endl;
        exit(-1);
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "socket creat erro!" << endl;
        exit(-1);
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    if (connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }
    for (;;)
    {
        cout << "================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "=============" << endl;
        cout << "choice: ";
        int choice;
        cin >> choice;
        cin.get();
        switch (choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get();
            cout << " userpassword:";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string req = js.dump();
            int len = send(clientfd, req.c_str(), strlen(req.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send login msg error:" << req << endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if (len == -1)
                {
                    cerr << "send login resopnse error" << endl;
                }
                else
                {
                    json responsjs = json::parse(buffer);
                    if (responsjs["errno"] != 0)
                    {
                        cout << responsjs["errmsg"] << endl;
                    }
                    else
                    {
                        g_currentUser.setID(responsjs["id"]);
                        g_currentUser.setName(responsjs["name"]);
                        g_currentUserFriendList.clear();
                        if (responsjs.contains("friends"))
                        {
                            vector<string> vec = responsjs["friends"];
                            for (auto &ch : vec)
                            {
                                json js = json::parse(ch);
                                User use(js["id"], js["name"], js["state"]);
                                g_currentUserFriendList.push_back(use);
                            }
                        }
                        g_currentUserGroupList.clear();
                        if (responsjs.contains("groups"))
                        {
                            vector<string> vec1 = responsjs["groups"];
                
                            for (auto &ch : vec1)
                            {
                                json grpjs = json::parse(ch);
                                Group group(grpjs["id"], grpjs["groupname"], grpjs["groupsec"]);
                                vector<string> v2 = grpjs["users"];
                                for (auto &n : v2)
                                {
                                    json jsuser = json::parse(n);
                                    GroupUser user;
                                    user.setID(jsuser["id"]);
                                    user.setName(jsuser["name"]);
                                    user.setRole(jsuser["role"]);
                                    user.setState(jsuser["state"]);
                                    group.getUser().push_back(user);
                                }
                                g_currentUserGroupList.push_back(group);
                            }
                        }
                        showCurrentUserData();
                        if (responsjs.contains("offlinemsg"))
                        {
                            vector<string> msg = responsjs["offlinemsg"];
                            for (auto &str : msg)
                            {
                                json js = json::parse(str);
                                if (ONE_CHAT_MESSAGE == js["msgid"])
                                {
                                    cout << js["time"].get<string>() << " [ " << js["id"] << " ] " << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
                                    continue;
                                }
                                else if (GROUP_CHAT_MSG == js["msgid"])
                                {
                                    cout << "这是一个群消息 [ " << js["groupid"] << " ] " << js["time"].get<string>() << " [ " << js["id"] << " ] " << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
                                    continue;
                                };
                            }
                        }
                        thread readTask(readTaskHandler, clientfd);
                        readTask.detach();
                        work=true;
                        mainMenu(clientfd);
                    }
                }
            }
            break;
        }
        case 2:
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:" << endl;
            cin.getline(name, 50);
            cout << "userpassword:";
            cin.getline(pwd, 50);
            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string req = js.dump();
            int len = send(clientfd, req.c_str(), strlen(req.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg error:" << req << endl;
            }
            else
            {
                char buf[1024] = {0};
                len = recv(clientfd, buf, 1024, 0);
                if (len == -1)
                {
                    cerr << "recv reg response error" << endl;
                }
                else
                {
                    json responsejs = json::parse(buf);
                    if (responsejs["errno"] != 0)
                    {
                        cerr << name << "is already exit,register error" << endl;
                    }
                    else
                    {
                        cout << name << "register success userid is" << responsejs["id"] << ",do not forget it!" << endl;
                    }
                }
            }
            break;
        }
        case 3:
        {
            close(clientfd);
            exit(0);
            break;
        }
        default:
            cout << "invalid input" << endl;
            break;
        }
    }
}
void readTaskHandler(int clientid)
{
    while (work)
    {
        char buffer[1024] = {0};
        int len = recv(clientid, buffer, 1024, 0);
        if (len == -1 || len == 0)
        {
            close(clientid);
            exit(-1);
        }
        json js = json::parse(buffer);
        if (ONE_CHAT_MESSAGE == js["msgid"])
        {
            cout << js["time"].get<string>() << " [ " << js["id"] << " ] " << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
        else if (GROUP_CHAT_MSG == js["msgid"])
        {
            cout << "这是一个群消息 [ " << js["groupid"] << " ] " << js["time"].get<string>() << " [ " << js["id"] << " ] " << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
    }
}

void showCurrentUserData()
{
    cout << "=============login user==============" << endl;
    cout << "current login user => id:" << g_currentUser.getID() << " name: " << g_currentUser.getName() << endl;
    cout << "------------friend list-----------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (auto &fr : g_currentUserFriendList)
        {
            cout << fr.getID() << " " << fr.getName() << " " << fr.getState() << endl;
        }
    }
    cout << "------------group list-----------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (auto &group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (auto &n : group.getUser())
            {
                cout << n.getID() << " " << n.getName() << " " << n.getState() << " " << n.getRole() << endl;
            }
        }
    }
    cout << "==============================" << endl;
}

unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friend"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}};
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};
void help(int clientid, string s )
{
    cout << "show command list >>> " << endl;
    for (auto &ch : commandMap)
    {
        cout << ch.first << ":" << ch.second << endl;
    }
    cout << endl;
}
void chat(int clientid, string s)
{
    json js;
    int idx = s.find(":");
    if (idx == -1)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    js["toid"] = atoi(s.substr(0, idx).c_str());
    js["msg"] = s.substr(idx + 1, s.size() - idx);
    js["msgid"] = ONE_CHAT_MESSAGE;
    js["id"] = g_currentUser.getID();
    js["name"] = g_currentUser.getName();
    js["time"] = getCurrentTime();
    int len = send(clientid, js.dump().c_str(), strlen(js.dump().c_str()), 0);
    if (len == -1)
    {
        cerr << "send addfriend msg erro ->" << js.dump() << endl;
    }
}
void addfriend(int clientid, string s)
{
    json js;
    js["friendid"] = atoi(s.c_str());
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getID();
    int len = send(clientid, js.dump().c_str(), strlen(js.dump().c_str()), 0);
    if (len == -1)
    {
        cerr << "send addfriend msg erro ->" << js.dump() << endl;
    }
}
void creategroup(int clientid, string s)
{
    json js;
    int idx = s.find(":");
    if (idx == -1)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    js["groupname"] = s.substr(0, idx);
    js["groupdesc"] = s.substr(idx + 1, s.size() - idx);
    js["msgid"] = CREAT_GROUP_MSG;
    js["id"] = g_currentUser.getID();
    int len = send(clientid, js.dump().c_str(), strlen(js.dump().c_str()), 0);
    if (len == -1)
    {
        cerr << "send createGroup msg erro ->" << js.dump() << endl;
    }
}

void addgroup(int clientid, string s)
{
    json js;
    js["groupid"] = atoi(s.c_str());
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getID();
    int len = send(clientid, js.dump().c_str(), strlen(js.dump().c_str()), 0);
    if (len == -1)
    {
        cerr << "send addgroup msg erro ->" << js.dump() << endl;
    }
}
void groupchat(int clientid, string s)
{
    json js;
    int idx = s.find(":");
    if (idx == -1)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    js["groupid"] = atoi(s.substr(0, idx).c_str());
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getID();
    js["msg"] = s.substr(idx + 1, s.size() - idx).c_str();
    js["name"] = g_currentUser.getName();
    js["time"] = getCurrentTime();
    int len = send(clientid, js.dump().c_str(), strlen(js.dump().c_str()), 0);
    if (len == -1)
    {
        cerr << "send addgroup msg erro ->" << js.dump() << endl;
    }
}
void quit(int clientid, string s)
{
}
void loginout(int clientid, string s)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getID();
    int len = send(clientid, js.dump().c_str(), strlen(js.dump().c_str()), 0);
    if (len == -1)
    {
        cerr << "send LOGINOUT msg erro ->" << js.dump() << endl;
    }
    else{
        work=false;
    }
}
void mainMenu(int clientid)
{
    help();
    char buffer[1024] = {0};
    while (work)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command !" << endl;
            continue;
        }
        it->second(clientid, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}