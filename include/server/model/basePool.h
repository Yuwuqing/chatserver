#ifndef BASEPOOL_H
#define BASEPOOL_H
#include<CommonConnectionPool.h>
#include<memory>
class base
{
protected:
    ConnectionPool *pool;
public:
    base()
    {
        pool = ConnectionPool::getConnectionPool();
    }

};

#endif
