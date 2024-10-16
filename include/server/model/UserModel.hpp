#ifndef USERMODEL_H
#define USERMODEL_H
#include "user.hpp"
class UserModel
{
private:
    /* data */
public:
    bool insert(User &user);
    void resetState();
    User query(int id);
    bool updateState(User use);

};


#endif