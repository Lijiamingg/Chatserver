#include<friendmodel.hpp>
#include "db.h"
#include <string>
using namespace std;
void Friendmodel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend(userid,friendid) values(%d, %d)",
            userid,friendid);
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}
vector<User> Friendmodel::query(int userid)
{
    vector<User> re;
    char sql[1024] = {0};
    sprintf(sql, "SELECT a.id,a.name,a.state FROM user a inner join friend b on a.id=b.friendid where b.userid=%d",
            userid);
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {   User fr;
                fr.setID(atoi(row[0]));
                fr.setName(row[1]);
                fr.setState(row[2]);
                re.push_back(fr);
            }
            mysql_free_result(res);
        }
    }
    return re;

}