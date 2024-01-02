#include "friendmodel.hpp"


// 添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend(userid, friendid) values(%d, %d)", userid, friendid);

    
    shared_ptr<MySQL> sp = pool->getConnection();
    if (sp)
    {
        sp->update(sql);
    }
}

// 返回用户好友列表  做两张表的联合查询，查询出id name offline/online
vector<User> FriendModel::query(int userid)
{
    // 1.组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select a.id, a.name, a.state from user a inner join friend b \
            on b.friendid = a.id where b.userid = %d", userid);

    vector<User> vec;
    shared_ptr<MySQL> sp = pool->getConnection();
    if(sp)
    {
        MYSQL_RES *res = sp->query(sql);
        if(res != nullptr)
        {
            // 把userid用户所有的离线消息放入vec中返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));//a.id
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}