#include "webserver.h"

// 初始化服务器核心资源
WebServer::WebServer()
{
    // 预分配客户端连接数组
    users = new HttpConn[MAX_FD];  // 管理与客户端的连接

    // 获取并设置服务器根目录(静态资源路径)
    char server_path[200];
    getcwd(server_path, 200);   // 获取当前工作目录
    char root[6] = "/root";     // 资源子目录
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);   // 拼接完整路径如: /home/project/root

    // 初始化客户端连接数组，包括sockfd, 和定时器
    client_data = new ClientData[MAX_FD];
}

// 释放资源
WebServer::~WebServer()
{
    // 关闭文件描述符
    close(m_epoll_fd);
    close(m_listen_fd);
    close(m_pipe_fd[1]);
    close(m_pipe_fd[0]);
    // 释放堆内存
    delete[] users;         // 释放连接对象数组
    delete[] client_data;   // 客户端连接数组
    delete   m_pool;        // 销毁线程池

    free(m_root);           // 释放根目录内存
}

// 初始化服务器配置参数
void WebServer::init(int port, string user, string password, string database_name,
              int log_write, int opt_linger, int trig_mode, int sql_num,
              int thread_num, int close_log, int actor_mode)
{
    // 网络配置
    m_port = port;
    m_opt_linger = opt_linger;      // 优雅关闭
    m_trig_mode = trig_mode;        // 触发模式
    m_actor_mode = actor_mode;    // 并发模式

    // 数据库配置
    m_user = user;
    m_password = password;
    m_database_name = database_name;
    m_sql_num = sql_num;

    // 线程池配置
    m_thread_num = thread_num;

    // 日志配置
    m_log_write = log_write;
    m_close_log = close_log;
}

// 设置触发模式组合
void WebServer::trig_mode()
{
    // 触发模式
    // 0: LT LT; 1: LT ET; 2 ET LT; 3: ET ET 
    m_listen_trig_mode = m_trig_mode / 2;  // 0 or 1: 0; 2 or 3: 1
    m_conn_trig_mode   = m_trig_mode % 2;  // 0 or 2: 0; 1 or 3: 1 
    // // LT + LT
    // if (m_trig_mode == 0)
    // {
    //     m_listen_trig_mode = 0;
    //     m_conn_trig_mode = 0;
    // }
    // // LT + ET 
    // else if (m_trig_mode == 1)
    // {
    //     m_listen_trig_mode = 0;
    //     m_conn_trig_mode = 1;
    // }
    // // ET + LT 
    // else if (m_trig_mode == 2)
    // {
    //     m_listen_trig_mode = 1;
    //     m_conn_trig_mode = 0;
    // }
    // // ET + ET 
    // else if (m_trig_mode == 3)
    // {
    //     m_listen_trig_mode = 1;
    //     m_conn_trig_mode = 1;
    // }
}

// 初始化日志
void WebServer::log_write()
{
    if (m_close_log == 0)   // 启用日志功能
    {
        const char *log_path = "./ServerLog";
        int buf_size = 2000;  // 日志缓冲区大小
        int split_lines = 800000;
        // 初始化日志
        if (m_log_write == 1)
            // get_instance() 返回Log的单例
            // init(filename, close_log, buf_size, split_lines, max_queue_size)
            // max_queue_size > 0, 日志异步写入
            Log::get_instance()->init(log_path, m_close_log, buf_size, split_lines, 800);
        else
            // max_queue_size = 0, 日志同步写入
            Log::get_instance()->init(log_path, m_close_log, buf_size, split_lines, 0);
    }
}

// 初始化数据库连接池
void WebServer::sql_pool()
{
    // 初始化数据库连接池
    m_conn_pool = ConnectionPool::GetInstance();
    m_conn_pool->init("localhost", m_user, m_password, m_database_name, 3306, m_sql_num,
                        m_close_log);
    
    // 初始化数据库读取表，预热数据库，加载用户表到内存
    // 从数据库连接池中取出一个连接，将数据库中 user 表信息加载至内存中(map<string,string> users)
    // LOG_INFO("sql_pool(%s, %s, %s)", m_user.c_str(), m_password.c_str(), m_database_name.c_str());
    users->init_mysql_result(m_conn_pool);
}

// 创建线程池
void WebServer::thread_pool()
{
    // 根据并发模式选择线程池类型
    // max_requests = 10
    LOG_INFO("ThreadPool<HttpConn>(m_thread_num: %d)",  m_thread_num);
    m_pool = new ThreadPool<HttpConn>(m_actor_mode, m_conn_pool, m_thread_num, 10);
}

// 初始化网络监听
void WebServer::event_listen()
{
    // 创建服务器监听连接请求套接字
    m_listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listen_fd >= 0);

    // 优雅关闭连接 (SO_LINGER?)
    if (m_opt_linger == 0)
    {
        struct linger tmp = {0, 1};     // 立即关闭，丢弃未发数据
        setsockopt(m_listen_fd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else if (m_opt_linger == 1)
    {
        struct linger tmp = {1, 1};     // 等待直到数据发送完毕或超时
        setsockopt(m_listen_fd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    // 设置服务器监听套接字的协议，地址，接口
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    // 设置端口复用，避免 TIME_WAIT
    setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    
    // 绑定并监听
    ret = bind(m_listen_fd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listen_fd, 5);
    assert(ret >= 0);

    // 初始化定时器(超时时间, 超过一段时间没有I/O事件则断开对应的客户端连接)
    utils.init(TIMESLOT);

    // epoll 创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    m_epoll_fd = epoll_create(5);
    assert(m_epoll_fd != -1);

    // 将连接socket 加入 epoll 内核事件表
    utils.addfd(m_epoll_fd, m_listen_fd, false, m_listen_trig_mode);
    HttpConn::m_epoll_fd = m_epoll_fd;

    // 创建信号通知管道
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipe_fd);
    assert(ret != -1);
    utils.set_nonblocking(m_pipe_fd[1]);
    utils.addfd(m_epoll_fd, m_pipe_fd[0], false, 0);

    // 设置信号处理
    utils.addsig(SIGPIPE, SIG_IGN);             // 忽略SIGPIPE(避免写关闭socket崩溃)
    utils.addsig(SIGALRM, utils.sig_handler, false);    // 定时器信号
    utils.addsig(SIGTERM, utils.sig_handler, false);    // 终止信号ctrl + c

    // 启动定时器
    alarm(TIMESLOT);

    // 工具类资源
    Utils::u_pipe_fd = m_pipe_fd;
    Utils::u_epoll_fd = m_epoll_fd;
}

// 为客户端新连接创建定时器
void WebServer::timer(int conn_fd, struct sockaddr_in client_address)
{
    // 初始化 HttpConn 对象
    users[conn_fd].init(conn_fd, client_address, m_root, m_conn_trig_mode, 
                        m_close_log, m_user, m_password, m_database_name);
    
    // 关联客户端数据 和 定时器
    client_data[conn_fd].address = client_address;
    client_data[conn_fd].sockfd = conn_fd;
    UtilTimer *timer = new UtilTimer;
    timer->user_data = &client_data[conn_fd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    client_data[conn_fd].timer = timer;
    // 加入定时器链表管理
    utils.m_timer_lst.add_timer(timer);
}

// 若有数据传输，则将过期时间往后延迟 3 个单位
// 并对事件定时器在链表上的位置进行调整
void WebServer::adjust_timer(UtilTimer *timer)
{
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;     // 超时时间延后 3 个时间单位
    utils.m_timer_lst.adjust_timer(timer);  // 调整链表位置

    LOG_INFO("%s", "adjust timer");
}

// 超时事件，从事件时间链表中删除
void WebServer::del_timer(UtilTimer *timer, int sockfd)
{
    // 执行回调函数，关闭客户端连接资源
    timer->cb_func(&client_data[sockfd]);
    if (timer)
    {
        // 从链表中移除
        utils.m_timer_lst.del_timer(timer);
        delete timer;   // 释放内存
    }

    LOG_INFO("close fd %d", client_data[sockfd].sockfd);
}

// 处理新来的客户端连接
bool WebServer::deal_client_data()
{
    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);

    // LT模式: 单次 accept 
    if (m_listen_trig_mode == 0)
    {
        // 建立与客户端的连接
        int conn_fd = accept(m_listen_fd, (struct sockaddr *)&client_address, &client_addr_len);
        if (conn_fd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }

        // 连接数超限
        if (HttpConn::m_user_count >= MAX_FD)
        {
            utils.show_error(conn_fd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        // 创建定时器，客户端连接资源与定时器关联
        timer(conn_fd, client_address);
    }
    // ET模式: 循环 accept 直到 EAGAIN
    else 
    {
        while (true)
        {
            int conn_fd = accept(m_listen_fd, (struct sockaddr *)&client_address, &client_addr_len);
            if (conn_fd == -1)
            {
                if (errno == EAGAIN) break; // 无新连接
                LOG_ERROR("Accept error: %s", strerror(errno));
                return false;
            }
            if (HttpConn::m_user_count >= MAX_FD)
            {
                utils.show_error(conn_fd, "Internal server busy");
                LOG_ERROR("%s", "Internal server busy");
                return false;
            }
            // 创建定时器，客户端连接资源与定时器关联
            timer(conn_fd, client_address);
        }
        return false;
    }
    return true;
}

// 处理信号事件
bool WebServer::deal_with_signal(bool &timeout, bool& stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(m_pipe_fd[0], signals, sizeof(signals), 0);

    if (ret == -1 || ret == 0)
        return false;
    else 
    {
        for (int i = 0; i < ret; ++ i)
        {
            switch(signals[i])
            {
            case SIGALRM:       // 定时信号，触发超时检测
            {
                timeout = true;
                break;
            }
            case SIGTERM:  // ctrl + c 终止信号，关闭服务器
            {
                stop_server = true;
                break;
            }
            }
        }
    }
    return true;
}

// 事件通知的模式ET/LT 和 事件处理模式 Reactor / Proactor
// ET/LT影响的是事件通知的时机和频率
// 而Reactor/Proactor决定了I/O操作的执行方式（同步或异步）
// Reactor通常配合非阻塞I/O和ET模式来提高效率
// 而Proactor可能依赖于操作系统的异步I/O支持 
// 本项目中同步I/O模拟proactor实现。
// 即主线程循环读取IO--并不是调用异步的IO函数，读取完成之后将http任务交给子线程处理.
// Proactor中子线程只需要处理业务逻辑，不需要处理IO事件.
void WebServer::deal_with_read(int sockfd)
{
    UtilTimer *timer = client_data[sockfd].timer;

    // reactor: 主线程仅注册事件，IO操作由子线程完成
    if (m_actor_mode == 1)
    {
        if (timer)      // 延续事件超时时间
            adjust_timer(timer);
        
        // 若检测到读事件，将读任务放入请求队列，
        // users + sockfd 对应与客户端连接的套接字，把该任务交给线程池中的一个子线程完成任务
        m_pool->append(users + sockfd, 0);

        // 等待子线程处理完成
        while (true)
        {
            if (users[sockfd].m_improv == 1)    // 等待子线程处理完任务
            {
                if (users[sockfd].m_timer_flag == 1)
                {
                    del_timer(timer, sockfd);
                    users[sockfd].m_timer_flag = 0;
                }
                else 
                {
                    users[sockfd].m_improv = 0;
                    break;
                }
            }
        }
    }
    // proactor: 主线程完成IO，子线程处理业务
    else 
    {
        if (users[sockfd].read_once())   // 当前主线程读取数据
        {
            LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

            // 若监测到读事件，将事件放入请求队列，交给线程池中的一个子线程完成业务逻辑
            m_pool->append_p(users + sockfd);

            if (timer)
                adjust_timer(timer);
        }
        else    // 读取失败，则关闭客户端连接
            del_timer(timer, sockfd);
    }
}

// 处理写事件
void WebServer::deal_with_write(int sockfd)
{
    UtilTimer *timer = client_data[sockfd].timer;

    // reactor: 主线程仅注册事件，IO操作由子线程完成
    if (m_actor_mode == 1)
    {
        // 更新事件的超时时间 （统一事件源: 本项目所有事件的形式都是文件描述符
        if (timer)
            adjust_timer(timer);
        
        // 添加写任务
        m_pool->append(users + sockfd, 1);

        // 等待子线程处理
        while (true)
        {
            if (users[sockfd].m_improv == 1)
            {
                if (users[sockfd].m_timer_flag)
                {
                    del_timer(timer, sockfd);
                    users[sockfd].m_timer_flag = 0;
                }
                users[sockfd].m_improv = 0;
                break;
            }
        }
    }
    // proactor: 主线程完成IO，子线程处理业务
    {
        // 同步写
        if (users[sockfd].write())
        {
            LOG_INFO("send data to the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

            if (timer)
                adjust_timer(timer);
        }
        else    // 发送失败则关闭
            del_timer(timer, sockfd);
    }
}

// 主事件循环
void WebServer::event_loop()
{
    bool timeout = false;       // 定时器触发标志
    bool stop_server = false;   // 服务器停止标志

    while (!stop_server)
    {
        // 等待事件 (阻塞调用)
        int number = epoll_wait(m_epoll_fd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        // 处理所有就绪事件
        for (int i = 0; i < number; ++ i)
        {
            int sockfd = events[i].data.fd;

            // 处理新到的客户端连接
            if (sockfd == m_listen_fd)
            {
                bool flag = deal_client_data();
                if (flag == false)
                    continue;
            }
            // 处理异常事件 (断开连接)
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 服务器关闭连接，移除对应的定时器
                UtilTimer *timer = client_data[sockfd].timer;
                del_timer(timer, sockfd);
            }
            // 处理信号
            else if (sockfd == m_pipe_fd[0] && (events[i].events & EPOLLIN))
            {
                bool flag = deal_with_signal(timeout, stop_server);
                if (flag == false)
                    LOG_ERROR("%s", "dealclientdata failure");
            }
            // 处理客户连接上接收的数据 -- 读事件
            else if (events[i].events & EPOLLIN)
            {
                deal_with_read(sockfd);
            }  
            // 处理写事件
            else if (events[i].events & EPOLLOUT)
                deal_with_write(sockfd);
        }

        // 定时处理超时连接
        if (timeout)
        {
            utils.time_handler();      // 调用计时器链表监测超时事件 tick()

            LOG_INFO("%s", "timer tick");

            timeout = false;
        }
    }
}