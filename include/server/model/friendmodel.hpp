#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>
#include "user.hpp"
using namespace std;
class Friendmodel
{
private:
    /* data */
public:
    void insert(int userid, int friendit);
    vector<User> query(int userid);

};


#endif