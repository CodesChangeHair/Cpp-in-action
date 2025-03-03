### http 连接处理类

根据状态转移,通过主从状态机封装了http连接类。其中,主状态机在内部调用从状态机,从状态机将处理状态和数据传给主状态机.

客户端发出http连接请求
从状态机读取数据,更新自身状态和接收数据,传给主状态机
主状态机根据从状态机状态,更新自身状态,决定响应请求还是继续读取.

#### epoll 

* I/O多路复用: 单个线程 or 进程 同时监控多个文件描述符的状态变化.
* 事件驱动: 仅处理活跃的连接，避免轮询所有连接的资源浪费.
  
##### epoll 的核心数据结构

1. 监控集合(红黑树)
监控通过`epoll_ctl()`注册的文件描述符, 每个节点包括文件描述符fd, 
, 监听事件类型 EPOLLIN/EPOLLOUT, 用户自定义数据epoll_data_t (后两个合在结构体epoll_event中)

2. 活跃事件队列(就绪列表)
动态存储已经触发的事件(就绪的I/O事件)

3. 回调机制
内核通过回调函数监测fd状态变化，当fd就绪时，自动将其事件添加到就绪链表.

`include <sys/epoll.h>`

##### epoll_create()

```
int epoll_create(int size);
```

创建一个指向 `epoll` 内核事件表的文教描述符，该文件描述符作为其他 `epoll` 系统
调用的第一个参数, 参数size不起作用.

创建一个红黑树存储待监控的文件描述符 和 就绪事件表 (存储活跃事件?)

#### epoll_ctl()

```
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

用于操作内核事件表监测的 文件描述符上的 事件: 注册、修改、删除.

event: 内核需要监听的事件.

```
struct epoll_event {
    __uint32_t events;  // Epoll events
    epoll_data_t data;  // User data variable
};
```

events描述事件类型，其中 epoll 事件类型有以下几种:

* EPOLLIN: 对应文件描述符可读(会有输入事件发生Input).
* EPOLLOUT: 对应文件描述符可写.
* EPOLLPRI: 紧急数据可读(带外数据).
* EPOLLERR: 对应的文件描述符发生错误.
* EPOLLHUP: 对应的文件描述符被挂断.
* EPOLLET:  将EPOLL设置为边缘触发(Edge Triggered) <--> 水平触发(Level Triggered)
* EPOLLONESHOT: 只监听一次事件，当监听完这次事件后，如果还需要监听这个事件，
  需要再次把这个 socket 加入到 EPOLL队列中.

##### epoll_wait()

```
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

epoll_wait() 等待监听文件描述符上事件的发生，返回就绪的文件描述符个数.

timeout: 超时时间.
* -1: 阻塞
* 0:  立即返回，非阻塞
* >0: 指定等待时间，毫秒

#### select / poll / epoll

##### 调用函数

select 和 poll 都是一个函数，epoll 是一组函数.

select 和 poll 将注册、监听、获取见过三合一，只调用select() 或 poll().
epoll 将功能分为三个函数: epoll_create(), epoll_ctl(), epoll_wait().

##### 文件描述符管理

* select 通过线性表描述文件描述符集合(位数组 fd_set)，文件描述符上限有限，一般是 1024.
* poll 通过链表描述(结构体数组 pollfd)，突破了文件描述符的上限, 仅受内存限制
* epoll 通过 红黑树 + 就绪链表 描述. 百万级别

##### 事件注册方式

* select 和 poll 每次传入全部文件描述符集合.
* 通过 epoll_ctl()动态维护注册事件.

##### 文件描述符的位置

* select 和 poll 在用户态管理文件描述符集合，每次调用时将集合临时拷贝进内核态.
* epoll 通过 epoll_create() 在内核中创建红黑树，将文件描述符交给内核管理.
  
##### 事件就绪判断

* select 和 poll 通过遍历文件描述符集合判断
* epoll 通过 每个文件描述符上的回调函数(中断函数)将就绪事件插入 事件就绪链表 list (动态),
epoll_wait() 观察 事件就绪链表 list 中是否有新的数据即可.

##### 就绪文件描述符索引

* select 和 poll 返回发送事件的文件描述符数量，需要遍历所有事件找到发生事件的文件描述符.
* epoll 返回发生事件的个数 和 结构体数组，结构体包含 socket 信息，因此只需要遍历
结构体数组即可确定事件发生的文件描述符.

##### 工作模式

* select 和 poll 只能工作在相对低效的 LT 水平触发 模式下
* epoll 可以工作在高效的 ET 边缘触发模式下，并且 epoll 支持 EPOLLONESHOT 事件，
该事件可以进一步减少 事件 被触发的次数.

##### 应用场景

###### select

* 跨平台需求 (Windows / Linux通用)
* 监控少量文件描述符(< 1000)
* 简单原型开发

###### epoll
* 高并发网络
* 长连接实时通信
* 大文件传输

#### ET, LT

* LT(水平触发，电平触发 Level Triggered): 只要文件描述符处于就绪状态，
epoll_wait()会持续通知. 如果事件未被处理完毕，下一次 epoll_wait()仍会触发.
例如套接字出现输入事件，输入缓冲区存储了数据. LT下 epoll_wait()会返回该数据. 若在下一次 epoll_wait()之前，输入缓冲区的数据没有被处理完毕，则该事件
仍会被返回. LT 编程简单，不需要一次处理完所有数据；可能会触发更多次系统调用. 

* ET 边缘触发: 仅在事件状态发生变化时返回(状态跳变的边缘)，若数据未处理完毕
也不会再返回事件. ET 复杂较复杂，需要循环写直到 EAGAIN; 减少了事件
通知次数，更高效.  

#### EPOLL ONESHOT

一个线程读取某个 socket 上的数据后开始处理数据，在处理过程中该 socket 上又有新的数据可读，
新的事件发生被 epoll 监听到，此时另一个线程被唤醒，两个线程同时处理一个 socket.

而我们期望的是一个 socket 连接在任意时刻都只被一个线程处理，通过 epoll_tcl 对该文件描述符
注册 EPOLLONESHOT 事件，此时 socket 只会在第一次有事件发生时被监听，只有一个线程处理该
socket. 当该线程处理完成后，通过 epoll_ctl 重置 EPOLLONESHOT 事件.

#### epoll 的回调函数 vs 信号处理函数

回调函数是在特定事件发送时被调用的函数，常见于事件驱动编程;
信号处理函数是进程接收到操作系统发送的信号时执行的函数.

##### 工作对比 

```
epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evebt)''
```

回调函数触发流程:
1. 网卡收到数据 --> 触发硬中断
2. 内核协议栈处理数据包
3. 检查 socket 等待队列中的 epoll 回调项
4. 执行 ep_poll_callback() 将事件加入就绪链表
5. 用户态通过 epoll_wait() 获取事件

```
void handler(int sig);
signal(SIGINT, handler);
```

信号处理函数流程:
1. 用户按下 Ctrl+C --> 内核生成 SIGINT
2. 暂停当前执行进程，保存上下文
3. 切换到信号处理栈
4. 执行 handler()
5. 恢复原程序执行

回调函数是主动的、结构化的事件处理机制

信号处理是被动的、应急性的中断响应


#### 优雅关闭 Graceful Connection Shutdown

网络编程中一种确保数据完整性和连接可靠性的技术. 通过协议和机制，
保证在关闭连接前完成所有必要的数据传输，避免数据丢失或连接重置. 

调用 close() 方法意味着完全断开连接. 例如 A 完成向 B的写数据，
调用 close()，此时 A 无法调用向B发送和接收数据的函数. 此时如果
B还有数据要发送给A， A也无法接收了. 这样关闭不太优雅. 

优雅关闭能够确保数据的完整性；避免端口和内存泄露。

优雅关闭指的是可以关闭 输入流 或 输出流, 半关闭 Half-close.

##### shutdown() 分步关闭

```
// 关闭写方向，不再发送数据，但可以接收数据
shutdown(sockfd, SHUT_WR); // SHUT_WR / SHUT_RD / SHUT_RDWR

// 继续读取剩余数据
while (recv(sockfd, buffer, sizeof(buffer), 0) > 0);

// 完全关闭连接
close(sockfd)
```

##### 设置 SO_LINGER 选项

```
struct linger {1, 5};  // 启动优雅关闭，最多等待 5 秒
setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &lgr, sizeof(lgr));
close(sockfd);  // 阻塞直到数据发送完毕或超时
```

struct linger 
{
  lgr_onoff;  1: 表示启动优雅关闭
  lgr_linger;  等待时间，超时后强制关闭
}


