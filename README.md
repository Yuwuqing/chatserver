# chatserver
可以工作在nginx tcp负载均衡环境中的集群聊天服务器和客户端代码源码 基于muduo库实现 redis mysql

编译方式
cd build/
rm -rf *
cmake ..
make

需要:
nginx tcp负载均衡
redis 基于发布-订阅的消息队列
mysql服务器和客户端
