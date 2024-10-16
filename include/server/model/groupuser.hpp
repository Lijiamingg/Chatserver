#ifndef GROUPUSER_H
#define GROUPUSER_H
#include<string>
#include <user.hpp>
using namespace std;
class GroupUser:public User
{
private:
    string role;
public:
    void setRole(string role){this->role=role;}
    string getRole(){return this->role;}
};


#endif