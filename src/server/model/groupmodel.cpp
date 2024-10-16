#include <groupmodel.hpp>
#include <db.h>
using namespace std;

bool Groupmodel::creatGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname,groupdesc) values('%s', '%s')",
            group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}
void Groupmodel::addGroup(int userid, int groupid, string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d,%d, '%s')",
            groupid, userid, role.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
vector<Group> Groupmodel::query(int userid)
{
    vector<Group> result;
    char sql[1024] = {0};
    sprintf(sql, "SELECT a.id, a.groupname,a.groupdesc FROM allgroup a inner join groupuser b on b.groupid=a.id WHERE b.userid = %d",
            userid);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group(atoi(row[0]), row[1], row[2]);
                result.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    for (auto &ch : result)
    {
        char sql[1024] = {0};
        sprintf(sql, "SELECT a.id, a.name,a.state,b.grouprole FROM user a inner join groupuser b on b.userid=a.id WHERE b.groupid = %d",
                ch.getId());
        MySQL mysql;
        if (mysql.connect())
        {
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) != nullptr)
                {
                    GroupUser groupuser;
                    groupuser.setID(atoi(row[0]));
                    groupuser.setName(row[1]);
                    groupuser.setState(row[2]);
                    groupuser.setRole(row[3]);
                    ch.getUser().push_back(groupuser);
                }
                mysql_free_result(res);
            }
        }
    }
    return result;
}
vector<int> Groupmodel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
        sprintf(sql, "SELECT userid FROM groupuser WHERE groupid = %d and userid != %d",
                groupid,userid);
        MySQL mysql;
        vector<int> result;
        if (mysql.connect())
        {
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) != nullptr)
                {
                    result.push_back(atoi(row[0]));
                }
                mysql_free_result(res);
            }
        }
        return result;
}