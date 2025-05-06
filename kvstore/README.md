### 框架

Network IO <---> Parse <--->    Engine 
网络IO            解析用户命令    数据(Key-Value)存储引擎 -- 数据结构

Network IO: reactor(epoll_entry) / 协程nytco(nytco_entry) / iouring

在输入和接收数据由IO完成，解析工作的实现为: 在IO框架中，在
IO之间插入解析, 数据引擎交互 操作.

kvstore 作为中介，与IO和数据引擎交互. 
IO <---> kvstore <--> engine

kvstore.h 声明需要实现的函数，
网络模块和engine模块include "kvstore.h"，实现具体函数.

### IO接收\r\n

在接收telnet数据时，例如测试COUNT命令时，出现"COUNT != COUNT"的情况;
以及GET KEY返回NOT EXIST，但此时`testcase`执行正确.
这是因为在使用telnet时，需要键盘按下回车发送数据，此时发送的数据结尾带有\r\n 
此时长度不相等，在网络接收数据时，
需要判断\r\n的情况，并将其改为\0，方便后续的C代码处理.

### TODO

engine:
    跳表
    B树

IO:
    iouring
    理解协程

考虑设计模式优化代码结构