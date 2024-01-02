#include "groupmodel.hpp"
#include "db.h"

// 加入群组
bool GroupModel::createGroup(Group &group)//group表
{
    // 1.组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into allgroup(groupname, groupdesc) values('%s', '%s')",
        group.getName().c_str(), group.getDesc().c_str());

    shared_ptr<MySQL> sp = pool->getConnection();
    if (sp)
    {
        if(sp->update(sql))
        {
            group.setId(mysql_insert_id(sp->getConnection()));
            return true;
        }
    }
    return false;
}

// 加入群组
void GroupModel::addGroup(int userid, int groupid, string role) //groupuser表
{
    // 1.组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into groupuser values(%d, %d, '%s')",
        groupid, userid, role.c_str());

    shared_ptr<MySQL> sp = pool->getConnection();
    if(sp)
    {
        sp->update(sql);
    }
}

// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid)
//返回的是用户所在的群组，每个群组都是group结构
//每个group结构需要填写四个信息
{
    /*
    1.先根据userid在groupuser表中查询出该用户所属的群组信息
    2.再根据群组信息，查询属于该群组所有用户的userid，并且和user表进行多表联合查询，查出用户的详细信息
    */
    char sql[1024]={0};
    sprintf(sql,"select a.id, a.groupname, a.groupdesc from allgroup a inner join groupuser b \
            on b.groupid = a.id where b.userid = %d", userid);//根据groupuser查allgroup表

    vector<Group> groupVec;
    shared_ptr<MySQL> sp = pool->getConnection();
    if(sp)
    {
        MYSQL_RES *res = sp->query(sql);
        if(res != nullptr)
        {
            // 查出userid所有的群组信息
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);//少压入一项，就是组的组员（自己的类）
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    // 查询群组的用户信息
    for(Group &group : groupVec)//再遍历每一项把组的组员加入到组中
    {
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from user a inner join groupuser b \
            on b.userid = a.id where b.groupid = %d", group.getId());//根据groupuser查user表 把组的组员加入到组中

        MYSQL_RES *res = sp->query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }        
    }
    return groupVec;
}


// 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    // 1.组装sql语句
    char sql[1024]={0};
    sprintf(sql,"select userid from groupuser where groupid = %d and userid != %d",groupid, userid);//groupuser

    vector<int> idVec;
    shared_ptr<MySQL> sp = pool->getConnection();
    if(sp)
    {
        MYSQL_RES *res = sp->query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}