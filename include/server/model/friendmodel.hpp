#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "user.hpp"
#include <vector>
#include<basePool.h>
using namespace std;
//Friend表 userid和friendid不同时一样，联合主键


// 维护好友信息的操作接口方法
class FriendModel : public base
{
public:
    // 添加好友关系
    void insert(int userid, int friendid);

    // 返回用户好友列表  做两张表的联合查询，查询出id name offline/online
    vector<User> query(int userid);
};

#endif // !FRIENDMODEL_H