#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include<group.hpp>
#include <vector>
using namespace std;
class Groupmodel
{
private:
    /* data */
public:
    bool creatGroup(Group &group);
    void addGroup(int userid,int groupid,string role);
    vector<Group> query(int userid);
    vector<int> queryGroupUsers(int userid,int groupid);
};


#endif