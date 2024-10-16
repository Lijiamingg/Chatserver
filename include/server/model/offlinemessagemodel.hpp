#ifndef OFFLINEMESSAGE_H
#define OFFLINEMESSAGE_H
#include<string>
#include <vector>
using namespace std;
class offlinemessagemodel
{
public:
    void insert(int userid,string msg);
    void remove(int userid);
    vector<string> query(int userid); 
    
};


#endif