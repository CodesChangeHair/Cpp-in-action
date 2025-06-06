### 半同步 / 半反应堆 线程池

使用一个工作队列将主线程(负责与客户端的I/O) 和 工作线程(负责http的解析和响应)
解耦合: 主线程向工作队列中添加任务，工作线程通过竞争工作队列取得任务并执行

#### 服务器编程基本框架

* I/O 单元: 处理客户端连接，读写网络数据. 本项目用一个主线程实现.
* 逻辑单元: 处理业务逻辑，本项目是处理 http 请求，用多个工作线程实现.
* 网络存储单元: 本地数据库和文件, 通过数据库连接池获取连接，用RAII方式管理连接.

#### 五种I/O模型.

通过小明等待水开(等待的I/O时间)为例.

* 阻塞 I/O: 调用某个函数后，等待函数返回，期间别的什么也不做，不停检查函数是否返回.
  小明在烧水后，盯着水壶看，等水开后才做别的事, 例如把热水倒在水瓶中.
* 非阻塞 I/O: 每隔一段时间检测 IO 事件是否就绪，若没就绪，在一段事件内可以做其他事情.
  非阻塞I/O执行系统调用总是立即返回，若事件没发生 或 出错，返回 -1, 可以根据 errno 判断,
  对于 accept(), recv() 和 send(), 事件未发生， errno 通常被设置为 eagain.
  小明在烧水后，隔一段时间看一次水壶开没开，没开就干别的事. 
* 信号驱动I/O: linux 用套接字进行信号驱动，安装一个IO信号处理函数，进程继续运行. 
  当IO事件就绪，进程收到SIGIO信号，处理IO事件.
  小明给购买了一个水烧开会鸣笛的水壶，这样在烧水后可以去干别的事，鸣笛后来处理水壶.
* IO复用: linux 用 select / epoll 函数实现 IO 复用模型，函数会使进程阻塞，但是和
  阻塞IO不同的是，这两个函数可以同时阻塞多个IO事件，可以同时检测多个文件描述符上是否有事件发生.
  小明雇了小王，让小王持续观察多个烧水壶，当有水壶烧开时，告诉小明哪些水壶需要被处理.
* 异步IO: linux 中，可以调用 aio_read() 函数告诉内核文件描述符缓冲区指针、缓冲区大小、
  文件偏移和通知方式，然后立即返回. 内核检测事件并完成IO事件，将数据拷贝到缓冲区，通知应用程序.
  小明雇了小王，小王不仅持续观察多个烧水壶，还能帮小明处理烧水壶，将热水倒在水瓶里，完成后
  通知小明水壶里的水已经烧开并被处理了.

同步: IO事件就绪 --> 进程处理IO事件.
异步: IO事件就绪并且由内核完成 --> 进程可以处理后续事件.

#### IO复用理解

假设同时有用户输入和连接请求两种IO事件，对于阻塞IO，我们得决定在一段时间内监听哪一种请求。
如果选择监听用户输入，则程序阻塞等待用户输入，而不能处理此时的连接请求。
IO复用解决这一问题，通过同时监听多种IO事件，阻塞等待，直到有事件发生返回，我们在返回时判断是那种IO事件，
做出对应的响应。

复用指的就是对一个东西用多次，反复用，这里就是用一个监听函数监听多次，监听多种事件。

TCP也是一种复用，在一段事件反复使用一个套接字，因为客户端和服务端保持了同步状态，所以
可以确认下一次请求是否属于同一个套接字，作为复用的条件。

#### 事件驱动编程模式

* reactor 模式: 主线程(I/O处理单元): 负责监听文件描述符上是否有事件发生，
如果有则立即通知工作进程(逻辑单元)，工作线程负责读写数据，接受新连接以及处理客户请求.
通常由同步I/O实现.

* proactor 模式: 主线程和内核负责读写数据、接受新连接等IO操作. 工作线程仅负责业务逻辑，如处理
  客户请求. 通常由异步I/O实现.

proactor 相比 reactor, 工作线程更加轻量，所有的I/O操作都由主线程和内核负责.

#### 同步I/O模拟preactor模式

由于异步I/O并不成熟，实际使用较少，本项目用同步I/O模拟实现preactor模式.

1. 主线程往 epoll 内核事件表注册 socket 上的 读就绪事件.
2. 主线程调用 `epoll_wait`等待读事件.
3. 当 socket 上有数据可读，`epoll_wait`通知主线程，主线程从socket循环读取数据,
没有数据可读，然后将读取到的数据封装成一个请求对象，放入请求队列.
4. 在请求队列上休眠等待的工作线程被唤醒，获得请求对象并处理客户请求，然后向
epoll 内核事件表中注册该 socket 上的 写就绪事件.
5. 主线程调用 `epoll_wait` 等待事件可写.
6. 当 socket 上有数据可写，`epoll_wait`通知主线程，主线程向 socket 上写入处理结果.

#### 半同步 / 半反应堆

##### 并发模式中的同步 和 异步

* 同步: 程序按代码指令流允许.
* 异步: 程序的执行需要由系统事件驱动.

##### 半同步 / 半异步模式 Half-sync / Half-async 

半同步/半异步模式是一种结合了同步与异步处理有点的编程模式，
在不牺牲同步操作精度的情况下，通过异步提高性能. 

* 半同步: 服务器的主线程或工作线程处理请求时，会进行同步操作.
* 半异步: 请求的实际处理通过异步完成. 服务器通常会使用一个队列
存储请求，然后交给工作线程执行.

在本项目中:

* 异步线程处理I/O事件: 监听到客户请求后，就将其封装成请求对象并插入到请求队列中.
  请求队列将通知某个工作在同步模式的工作线程来读取并处理请求对象.
* 同步线程处理客户逻辑

##### 半同步 / 半反应堆 Half-sync / Half-reactor 

###### 反应堆模式 Reactor Pattern

反应堆模式是一种事件驱动的模式，主要应用于处理大量的I/O事件，核心思想:
* 反应器 Reactor: 负责监听多个 I/O事件，并将事件分发给相应的处理程序.
* 处理程序 Handlers: 具体的事件处理逻辑，比如读取写入数据等.
  
##### 工作流程

反应堆模式通常处理I/O事件的调度与分发，但处理请求时可能会涉及到同步的工作流程，
例如数据库访问，计算密集型任务.

1. 主线程作为异步线程，负责监听所有 socket 上的事件.
2. 当监听socket上有读事件时，主线程创建新的连接，向 epoll 内核事件表中注册该 socket 的读写事件.
3. 如果连接socket上有读写事件发生，主线程从 socket 接受数据，并将数据封装成请求放入请求队列中.
4. 所有工作线程睡眠在请求队列上，当有任务到来，通过竞争获得任务的接管权.

#### 静态成员变量

##### 声明周期

静态成员属于类而不是属于实例，存储在 静态存储区(全局数据区),
静态成员在程序启动时初始化，程序结束时销毁.

##### 类内声明类外定义

类内声明：仅声明静态成员的符号，不会分配内存.
类外定义: 在一个编译单元(通常是.cpp文件)中定义静态成员, 以分配内存并初始化.

##### 为什么需要在类外初始化?

* 避免重复定义: 若类定义在头文件中，类内初始化可能导致多个编译单元重复定义静态变量.
* 显式控制初始化顺序: 类外定义允许开发者在特定位置显式管理初始化.
* C++17允许通过 `inline` 在类内直接初始化静态成员.

#### 静态成员函数

* 静态成员函数可以直接访问静态成员变量，不能直接访问普通成员变量，
但可以通过参数传递的方式访问普通成员变量.

* 普通成员函数可以访问普通成员变量，也可以访问静态成员变量.

* 静态成员函数没有 `this` 指针. 非静态成员为对象单独维护，
每个对象对应一个`this`指针；而静态成员函数由类管理，是对象的
共享函数，无法区分属于哪个对象，因此不能直接访问普通变量，也没有`this`指针.

#### pthread_create 函数陷阱

```
#include <pthread.h>
int pthread_create(pthread_t *thread_id,        // 返回新生成的线程id
                   const pthread_attr_t *attr,  // 指向线程属性的指针，通常为NULL
                   void* (*start_routine)(void *),  // 线程函数指向的函数指针
                   void *arg,                       // start_routine()中的参数
)
```

`pthread_create`函数的第三个参数为函数指针，指向处理线程的函数的地址. 如果线程函数是类成员函数时，需要将其设置为静态成员函数.

若线程函数为类成员函数(非静态), 则`this`指针会作为默认参数被传进函数中，从而和线程函数参数
`void*`不匹配，不能通过编译.

而静态成员函数就没有这个问题，不会默认传递 `this` 指针.

