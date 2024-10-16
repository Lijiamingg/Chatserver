#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType
{
    LOGIN_MSG = 1, // 登录消息
    LOGIN_MSG_ACK,
    LOGINOUT_MSG,
    REG_MSG,     // 注册消息
    REG_MSG_ACK,
    ONE_CHAT_MESSAGE,
    ADD_FRIEND_MSG,//添加好友消息
    CREAT_GROUP_MSG,
    ADD_GROUP_MSG,
    GROUP_CHAT_MSG,
};

#endif