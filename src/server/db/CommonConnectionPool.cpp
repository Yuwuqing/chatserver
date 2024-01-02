#include "CommonConnectionPool.h"



// 线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool; 
	return &pool;
}


// 连接池的构造
ConnectionPool::ConnectionPool()
{
	// 创建初始数量的连接
	for (int i = 0; i < _initSize; ++i)
	{
		MySQL *p = new MySQL();
		p->connect();
		p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
		_connectionQue.push(p);
		_connectionCnt++;
	}
	//系统启动时候做的，当前只有主线程

	// 启动一个新的线程，作为连接的生产者 linux thread => pthread_create
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));//线程函数都是c接口
	produce.detach();

	// 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}

// 运行在独立的线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTask()//成员方法，方便访问成员变量
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty())
		{
			cv.wait(lock); // 队列不空，此处生产线程进入等待状态
		}

		// 连接数量没有到达上限，继续创建新的连接
		if (_connectionCnt < _maxSize)
		{
			MySQL *p = new MySQL();
			p->connect();
			p->refreshAliveTime(); // 自己的p刷新一下开始空闲的起始时间
			_connectionQue.push(p);
			_connectionCnt++;
		}

		// 通知消费者线程，可以消费连接了
		cv.notify_all();
	}
}

// 给外部提供接口，从连接池中获取一个可用的空闲连接
shared_ptr<MySQL> ConnectionPool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);//队列不是线程安全的 
	while (_connectionQue.empty())
	{
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
            //超时醒来的
			if (_connectionQue.empty())
			{
				cout<<("获取空闲连接超时了...获取连接失败!")<<endl;
					return nullptr;
			}
		}
	}


	shared_ptr<MySQL> sp(_connectionQue.front(), 
		[&](MySQL *pcon) {
		unique_lock<mutex> lock(_queueMutex);
		pcon->refreshAliveTime(); 
		_connectionQue.push(pcon);
	});
	
	_connectionQue.pop();
	cv.notify_all();  // 消费完连接以后，通知生产者线程检查一下，如果队列为空了，赶紧生产连接（其他人消费！）
	
	return sp;
}

// 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		// 通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		// 扫描整个队列，释放多余的连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			MySQL *p = _connectionQue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p; // 调用~Connection()释放连接
			}
			else
			{
				break; // 队头的连接没有超过_maxIdleTime，其它连接肯定没有
			}
		}
	}
}