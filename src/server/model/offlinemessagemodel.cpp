#include "offlinemessagemodel.hpp"
#include "db.h"
void offlinemessagemodel::insert(int userid, string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage(userid,message) values(%d,'%s')",
            userid,msg.c_str());
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}
void offlinemessagemodel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d",
            userid);
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}
vector<string> offlinemessagemodel::query(int userid)
{
    char sql[1024] = {0};
    vector<string> result;
    sprintf(sql, "SELECT message FROM offlinemessage WHERE userid = %d",
            userid);
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                result.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return result;
}