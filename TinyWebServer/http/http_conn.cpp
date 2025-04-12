#include "http_conn.h"

#include <mysql/mysql.h>
#include <fstream>

//定义http响应的一些状态信息
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

Locker m_lock;
map<string, string> users;

// 从数据库连接池中获取一个连接，通过这个连接获取数据库中 user 表的
// 数据，存入内存中 (map<string, string> users中)
void HttpConn::init_mysql_result(ConnectionPool *conn_pool)
{
    // 从连接池中取出一个连接
    MYSQL *mysql = NULL;
    ConnectionRAII mysql_conn(&mysql, conn_pool);

    // 在数据库 user 表 中检索 (username, password)
    if (mysql_query(mysql, "SELECT username, password FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
        exit(1);
    }
    
    if (!mysql)
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
        exit(1);
    }
    
    // 从表中检索完整的结果表
    MYSQL_RES *result = mysql_store_result(mysql);

    // 返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    // 返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    // 从结果集中获取所有行，存入 map<string, string>users 中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        string name(row[0]);
        string passwd(row[1]);
        users[name] = passwd;
    }
}

// 对文件描述符设置非阻塞
int set_nonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 将 fd 注册到 epoll 内核事件表中，ET模式，选择开启 EPOLL ONESHOT
void addfd(int epoll_fd, int fd, bool one_shot, int trig_mode)
{
    epoll_event event;      // epoll 事件
    event.data.fd = fd;     // 绑定文件描述符

    if (trig_mode == 1)
        // EPOLLIN: 监听可读事件，当 fd 可读时触发
        // EPOLLRDHUP: 对端关闭连接时触发
        // EPOLLET: ET边缘触发模式
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else    // 默认为水平触发LT模式
        event.events = EPOLLIN | EPOLLRDHUP;
    
    // EPOLLONESHOT: 保证 socket 连接在任意时刻只被一个线程处理
    if (one_shot)
        event.events |= EPOLLONESHOT;
    
    // 向 epoll 实例(epoll_fd)中添加(CTL_ADD)需要监听的文件描述符fd,
    // event指定epoll事件(读/写/ET...)
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);

    set_nonblocking(fd);
}

// 从 epoll 内核事件表中删除 文件描述符, 并关闭文件描述符
void removefd(int epoll_fd, int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

// 将事件重置为 EPOLL ONESHOT
void modfd(int epoll_fd, int fd, int events, int trig_mode)
{
    epoll_event event;
    event.data.fd = fd;

    if (trig_mode == 1) // 边缘触发
        event.events = events | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else 
        event.events = events | EPOLLONESHOT | EPOLLRDHUP;
    
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

// 类外初始化静态成员变量
int HttpConn::m_user_count = 0;
int HttpConn::m_epoll_fd = -1;

// 关闭与客户端的连接
void HttpConn::close_conn(bool real_close)
{
    if (real_close && (m_sockfd != -1))
    {
        printf("close client %d\n", m_sockfd);
        removefd(m_epoll_fd, m_sockfd);
        m_sockfd = -1;
        -- m_user_count;
    }
}

// 初始化连接
void HttpConn::init(int sockfd, const sockaddr_in &addr, char *root, int trig_mode,
                    int close_log, string user, string password, string sql_name)
{
    m_sockfd = sockfd;
    m_address = addr;

    addfd(m_epoll_fd, sockfd, true, m_trig_mode);  // 这里直接用m_trig_mode ?

    m_doc_root = root;
    m_trig_mode = trig_mode;
    m_close_log = close_log;

    strcpy(m_sql_user, user.c_str());
    strcpy(m_sql_password, password.c_str());
    strcpy(m_sql_name, sql_name.c_str());

    init();
}

// 初始化新接受的连接
void HttpConn::init()
{
    m_mysql = NULL;
    m_bytes_to_send = 0;
    m_bytes_have_send = 0;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    m_cgi = 0;
    m_state = 0;

    m_timer_flag = 0;
    m_improv = 0;

    memset(m_read_buf, 0, READ_BUFFER_SIZE);
    memset(m_write_buf, 0, WRITE_BUFFER_SIZE);
    memset(m_real_file, 0, FILENAME_LEN);
}

// 从状态机(相对于主状态机)，用于分析 http 请求的一行
// 返回值为行的读取状态: LINE_OK, LINE_BAD, LINE_OPEN
// http 报文中，每一行数据以 \r\n 结束，空行则仅仅是\r\n 
// 可以通过查找 \r\n 将报文拆解成单独的行进行解析
// 从状态机负责从读缓冲区读取数据，将每一行数据末尾的\r\n 替换为 \0\0 
// 并更新 读缓冲区 m_read_buf 读取的位置 m_checked_idx
HttpConn::LINE_STATUS HttpConn::parse_line()
{
    char temp;
    for (; m_checked_idx < m_read_idx; ++ m_checked_idx)
    {
        temp = m_read_buf[m_checked_idx];
        if (temp == '\r')
        {
            // 到达缓冲区字符结尾 
            if ((m_checked_idx + 1) == m_read_idx)
                return LINE_OPEN;
            else if (m_read_buf[m_checked_idx + 1] == '\n')
            {
                m_read_buf[m_checked_idx ++] = '\0';
                m_read_buf[m_checked_idx ++] = '\0';
                return LINE_OK;
            }
            else  // 语法错误
                return LINE_BAD;
        }
        // 判断上一个字符是否是 \r, 一般在上次读取时读到\r 遇到了 buffer 末尾，没有完整接收(LINE_OPEN)
        // 会遇到这种情况
        else if (temp == '\n')
        {
            if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r')
            {
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx ++] = '\0';
                return LINE_OK;
            }
            else 
                return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

// 循环读取客户数据，直到无数据 或 对方关闭连接
bool HttpConn::read_once()
{
    // m_read_idx: 读取的位置 大于 缓冲区大小 
    // 如果读缓冲区已满，退出
    if (m_read_idx >= READ_BUFFER_SIZE)
        return false;
    
    int bytes_read = 0;

    // LT 读取数据, 只要数据仍然可读，epoll就会不断触发，因此可以直接使用阻塞读取,
    // 数据可以分多次读取
    // 适用于阻塞I/O，避免CPU轮询
    if (m_trig_mode == 0)
    {
        // 从套接字接收数据，存储在m_read_buf缓冲区中,
        // 从 m_read_buf + m_read_idx 开始，最多存储 READ_BUFFER_SIZE - m_read_idx 个字符
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        m_read_idx += bytes_read;  // 更新读取字符数

        if (bytes_read <= 0)
            return false;
        else 
            return true;
    }
    // 非阻塞 ET 工作模式下，epoll 只会在数据初次可读(跳变的边缘)时触发一次事件
    // 因此需要循环读取直到数据被读完，一次触发，也就需要一次处理完毕
    // 防止数据残留导致读不到完整数据
    // 适用于非阻塞IO，减少epoll调用，提高性能
    {
        while (true)
        {
            bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            if (bytes_read == -1)
            {
                // 这两错误代码表示 非阻塞读取中 当前无数据可读
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else 
                    return false;
            }
            else if (bytes_read == 0)
                return false;
            else 
                m_read_idx += bytes_read;
        }
        return true;
    }
}

// 主状态机的初始状态是 CHECK_STATE_REQUESTLINE，解析 http 的请求行
// 解析 http 请求行, 获得请求方法、目标url 和 http 版本号
// 解析完成后 parse_request_line() 将主状态机的状态更改为 CHECK_STATE_HEADER
// 告诉主状态机继续请求头的解析
HttpConn::HTTP_CODE HttpConn::parse_request_line(char *text)
{
    // HTTP 报文中，请求行用来说明请求类型，要访问的资源 以及 HTTP版本
    // GET /xxx.png HTTP/1.1  POST / HTTP/1.1 
    // 其中各部分用 \t 或 空格 分隔
    // strpbrk() 返回请求行中首次含有 \t 或 空格 的位置并返回
    m_url = strpbrk(text, " \t");

    // 如果没有空格 或 \t, 则报文格式有误
    if (!m_url) 
        return BAD_REQUEST;
    
    // 将第一个空格或\t改为字符串终止字符\0, 用于将前面的数据作为一个字符串取出 
    *m_url ++ = '\0';

    // 取出第一段数据(被空格或\t分隔), 确定请求方式
    char *method = text;
    if (strcasecmp(method, "GET") == 0)
        m_method = GET;
    else if (strcasecmp(method, "POSE") == 0)
    {
        m_method = POST;
        m_cgi = 1;          // ?
    } 
    else
        return BAD_REQUEST;
    
    // m_url 跳过后续的所有空格或\t, 来到第二个字符串首位 -- 请求资源字符串的首位
    m_url += strspn(m_url, " \t");

    // 与判断请求方式的逻辑相同，判断版本号
    m_version = strpbrk(m_url, " \t");
    if (!m_version)
        return BAD_REQUEST;
    
    // 将中间的请求资源字符分割出来
    *m_version ++ = '\0';
    
    m_version += strspn(m_version, " \t");
    if (strcasecmp(m_version, "HTTP/1.1") != 0)
        return BAD_REQUEST;
    
    // 对请求资源进行额外的判断
    // 有些报文的请求资源中会带有http://，这里需要对这种情况进行单独处理
    if (strncasecmp(m_url, "http:://", 7) == 0)
    {
        m_url += 7;
        m_url = strchr(m_url, '/');
    }
    // 有些报文的请求资源中会带有https://，这里需要对这种情况进行单独处理
    if (strncasecmp(m_url, "https:://", 8) == 0)
    {
        m_url += 8;
        m_url = strchr(m_url, '/');
    }

    // 资源以 / 开始
    if (!m_url || m_url[0] != '/')
        return BAD_REQUEST;
    
    // 资源字符串仅仅为 /, 返回判断页面judge.html
    if (strlen(m_url) == 1)
        strcat(m_url, "judge.html");
    
    // 请求处理完毕，将主状态机状态转移为处理请求头
    m_check_state = CHECK_STATE_HEADER;

    return NO_REQUEST;
}

// 解析 http 请求的一个头部信息
// 这里同时解析请求头和空行，空行的第一个字符是\0 (被从状态机将\r修改后)
// 若是空行，则判断content-length是否为0，即是否包含请求内容
// 0表示没有请求内容，为GET请求；否则有请求内容，为POST请求
// 若解析请求头部，则主要解析connection字段 和 content-length字段
// 在登录和注册时，为避免用户名和密码直接暴露在URL中(GET)，使用 POST
// 请求可以将 用户名和密码添加在消息体中封装起来
HttpConn::HTTP_CODE HttpConn::parse_headers(char *text)
{
    // 空行
    if (text[0] == '\0')
    {
        // POST 请求，将主状态机状态转移为 消息体处理状态
        if (m_content_length != 0)
        {
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        else    // GET 请求
            return GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection", 11) == 0)
    {
        text += 11;     
        // 跳过空格和\t
        text += strspn(text, " \t");
        // 如果是长连接，设置 linger 标志为 true
        if (strcasecmp(text, "keep-alive") == 0)
            m_linger = true;
    }
    else if (strncasecmp(text, "Content-length:", 15) == 0)
    {
        text += 15;
        text += strspn(text, " \t");
        m_content_length = atol(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    }
    else 
        LOG_INFO("opp! unkown header: %s", text);
    return NO_REQUEST;
}

// 主状态机状态: CHECK_STATE_CONTENT
// 仅用于解析 POST 请求，保存 post 请求消息体，为后面的登录和注册做准备
// 判断 http 请求是否被完整读入
HttpConn::HTTP_CODE HttpConn::parse_content(char *text)
{
    // 仍然有没有解析的内容
    if (m_read_idx >= (m_content_length + m_checked_idx))
    {
        text[m_content_length] = '\0';
        // POST 请求最后为输入的用户名和密码
        m_string = text;
        return GET_REQUEST;
    }
    else 
        return NO_REQUEST;
}

// m_start_line 已经解析的字符的最大索引
// get_line() 指向未处理的第一个字符
// char *get_line()
// {
//     return m_read_buf + m_start_line;
// }

// 循环处理: 从状态每次从请求报文中取出一行, 将每一行结尾的 \r\n 替换为\0\0 方便主状态机处理
// 主状态机处理获得的报文
HttpConn::HTTP_CODE HttpConn::process_read()
{
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = 0;

    // line_status == parse_line()) == LINE_OK: 处理请求头/请求行时, 循环条件是从状态机取出了完整的一行
    // 对于POST，需要处理消息体，每一行不一定是完整的一行(以 \r\n 结尾)
    // 此时循环条件为 m_check_state == CHECK_STATE_CONTENT
    // 加上&& line_status == LINE_OK 是为了在处理完消息体或出错时可以方便退出循环
    // 因为CHECK_STATE_CONTENT是主状态机的最后一个状态，主状态机的状态会停留在这个状态不变
    while ((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) || 
           (line_status == parse_line()) == LINE_OK)
    {
        text = get_line();          // 获取未解析的第一个字符
        m_start_line = m_checked_idx; // 

        LOG_INFO("%s", text);

        // 主状态机的三种状态
        switch (m_check_state)
        {
        case CHECK_STATE_REQUESTLINE:
        {
            ret = parse_request_line(text);
            if (ret == BAD_REQUEST)
                return BAD_REQUEST;
            break;
        }
        case CHECK_STATE_HEADER:
        {
            ret = parse_headers(text);
            if (ret == BAD_REQUEST)
                return BAD_REQUEST;
            // 已经完整解析 GET 请求(无消息体)，跳转至响应报文函数
            else if (ret == GET_REQUEST)
                return do_request();
            break;
        }
        case CHECK_STATE_CONTENT:
        {
            ret = parse_content(text);

            // 完整解析POST请求后，跳转到报文响应函数
            if (ret == GET_REQUEST)
                return do_request();
            else 
                line_status = LINE_OPEN;    // 处理POST消息体时出错，更改line_status以退出while循环
            break;
        }
        default:
            return INTERNAL_ERROR;
        }
    }
}

// 将网络目录和 url 文件拼接，通过stat()判断文件属性 
// 为了提高访问速度，通过 mmap()进行映射，将普通文件映射到内存逻辑地址
HttpConn::HTTP_CODE HttpConn::do_request()
{
    // m_doc_root 为服务器根目录，存放资源 和 跳转的 html 文件
    strcpy(m_real_file, m_doc_root);
    int len = strlen(m_doc_root);
    //printf("m_url:%s\n", m_url);

    // 首个 / 的位置
    const char *p = strrchr(m_url, '/');

    // 处理CGI请求，实现登录和注册校验
    if (m_cgi == 1 && (*(p + 1) == '2' || *(p + 1) == '3'))
    {
        // 根据 URL 的第二个字符判断操作类型 2-注册，3-登录
        char flag = m_url[1];

        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/");        
        strcat(m_url_real, m_url + 2);      // 拼接路径 跳过操作标识符
        strncpy(m_real_file + len, m_url_real, FILENAME_LEN - len - 1); // 组合出完整路径
        free(m_url_real);

        //将用户名和密码提取出来
        //user=123&password=123
        char name[100], password[100];
        int i;
        // i = 5, 跳过user=
        for (i = 5; m_string[i] != '&'; ++ i)
            name[i - 5] = m_string[i];
        name[i - 5] = '\0';

        int j = 0;
        // i = i + 10 跳过 &password=
        for (i = i + 10; m_string[i] != '\0'; ++i, ++ j)
            password[j] = m_string[i];
        password[j] = '\0';

        // 注册逻辑
        if (*(p + 1) == '3')
        {
            //如果是注册，先检测数据库中是否有重名的
            //没有重名的，进行增加数据
            char *sql_insert = (char *)malloc(sizeof(char) * 200);
            // 构造 SQL 插入 insert 语句
            strcpy(sql_insert, "INSERT INTO user(username, password) VALUES(");
            strcat(sql_insert, "'");
            strcat(sql_insert, name);
            strcat(sql_insert, "', '");
            strcat(sql_insert, password);
            strcat(sql_insert, "')");

            // 查询用户名是否已经存在
            // 如果 用户名 不在 map 集合中 -- 初始化 mysql 时将 user 表中的数据加入了内存中
            if (users.find(name) == users.end())    
            {
                m_lock.lock();
                int res = mysql_query(m_mysql, sql_insert);   // 执行 SQL
                users.insert(pair<string, string>(name, password));  // 更新 users(map) 内存缓存
                m_lock.unlock();
                
                // 操作处理结果
                if (!res)   // 插入成功则跳转至登录页面
                    strcpy(m_url, "/log.html");
                else        // 失败跳转至错误页面
                    strcpy(m_url, "/registerError.html");
            }
            else        // 用户名已经存在
                strcpy(m_url, "/registerError.html");
            free(sql_insert);
        }
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        else if (*(p + 1) == '2')
        {
            // 价差用户名是否存在 且 密码是否匹配 (在内存中对比)
            if (users.find(name) != users.end() && users[name] == password)
                strcpy(m_url, "/welcome.html");     // 登录成功
            else
                strcpy(m_url, "/logError.html");    // 登录失败
        }
    }

    // 静态页面路由处理
    if (*(p + 1) == '0')    // 注册页面
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/register.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if (*(p + 1) == '1')   // 登录页面
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/log.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if (*(p + 1) == '5')   // 图片页面
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/picture.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if (*(p + 1) == '6')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/video.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if (*(p + 1) == '7')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/fans.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else    // 直接访问文件资源
        strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);

    // 获取文件状态信息
    if (stat(m_real_file, &m_file_stat) < 0)
        return NO_RESOURCE; // 文件不存在

    if (!(m_file_stat.st_mode & S_IROTH))
        return FORBIDDEN_REQUEST;

    if (S_ISDIR(m_file_stat.st_mode))
        return BAD_REQUEST;

    int fd = open(m_real_file, O_RDONLY);
    m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

void HttpConn::unmap()
{
    if (m_file_address)
    {
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = 0;
    }
}

// 服务器子线程调用 process_write() 完成响应报文，随后注册 epollout 事件.
// 服务器主线程负责监听 I/O事件，监测到 写事件后，调用 write() 函数将响应报文发送给游览器.

bool HttpConn::write()
{
    int temp = 0;

    // 需要发送的数据长度为0，响应报文为空
    if (m_bytes_to_send == 0)
    {
        // 重新监听该 socket 
        modfd(m_epoll_fd, m_sockfd, EPOLLIN, m_trig_mode);
        init();     // 初始化新接受的连接
        return true;
    }

    while (true)
    {
        // 将响应报文的内容通过套接字发送给游览器端
        // 通过 iovec 向量机制写入 (已经由服务器子线程完成)
        temp = writev(m_sockfd, m_iovec, m_iovec_count);

        if (temp < 0)
        {
            if (errno == EAGAIN)    // 对应的事件还未发生?
            {
                // 重新注册写事件
                modfd(m_epoll_fd, m_sockfd, EPOLLOUT, m_trig_mode);
                return true; 
            }
            else 
            {
                // 发送失败，取消内存映射
                unmap();
                return false;
            }
        }

        // temp >= 0

        m_bytes_have_send += temp;  // 更新已经发送的字节数
        m_bytes_to_send -= temp;    // 更新剩余发送的字节
        if (m_bytes_to_send >= m_iovec[0].iov_len)  
        {
            m_iovec[0].iov_len = 0;
            m_iovec[1].iov_base = m_file_address + (m_bytes_have_send - m_write_idx);
            m_iovec[1].iov_len = m_bytes_have_send;
        }
        else 
        {
            m_iovec[0].iov_base = m_write_buf + m_bytes_have_send;
            m_iovec[0].iov_len  = m_iovec[0].iov_len - m_bytes_have_send;
        }

        if (m_bytes_to_send <= 0)
        {
            unmap();
            modfd(m_epoll_fd, m_sockfd, EPOLLIN, m_trig_mode);

            if (m_linger)
            {
                init();
                return true;
            }
            else 
                return false;
        }
    }
}

bool HttpConn::add_response(const char *format, ...)
{
    if (m_write_idx >= WRITE_BUFFER_SIZE)
        return false;
    
    va_list arg_list;
    va_start(arg_list, format);

    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - m_write_idx - 1, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - m_write_idx - 1))
    {
        va_end(arg_list);
        return false;
    }
    else 
    {
        m_write_idx += len;
        va_end(arg_list);

        LOG_INFO("request: %s", m_write_buf);

        return true;
    }
}

bool HttpConn::add_status_line(int status, const char *title)
{
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool HttpConn::add_content_length(int content_len)
{
    return add_response("Content-Length:%d\r\n", content_len);
}

bool HttpConn::add_headers(int content_len)
{
    return add_content_length(content_len) && add_linger() &&
           add_blank_line();
}

bool HttpConn::add_content_type()
{
    return add_response("Content-Type:%s\r\n", "text/html");
}

bool HttpConn::add_linger()
{
    return add_response("Connection:%s\r\n", (m_linger == true) ? "keep-alive" : "close");
}

bool HttpConn::add_blank_line()
{
    return add_response("%s", "\r\n");
}

bool HttpConn::add_content(const char *content)
{
    return add_response("%s", content);
}

// 根据 do_request() 的返回状态，服务器子线程调用 process_write() 向 m_write_buf 中写入响应报文
// add_status_line() 添加状态行: http1.1 / 状态码 状态信息
// add_headers()添加消息头，每部调用 add_content_length() 和 add_linger()
// add_blank_line() 添加空行
bool HttpConn::process_write(HTTP_CODE ret)
{
    switch (ret)
    {
    case INTERNAL_ERROR:
    {
        add_status_line(500, error_500_title);
        add_headers(strlen(error_500_form));
        if (!add_content(error_500_form))
            return false;
        break;
    }
    case BAD_REQUEST:
    {
        add_status_line(404, error_404_title);
        add_headers(strlen(error_404_form));
        if (!add_content(error_404_form))
            return false;
        break;
    }
    case FORBIDDEN_REQUEST:
    {
        add_status_line(403, error_403_title);
        add_headers(strlen(error_403_form));
        if (!add_content(error_403_form))
            return false;
        break;
    }
    case FILE_REQUEST:
    {
        add_status_line(200, ok_200_title);
        if (m_file_stat.st_size != 0)   // 文件大小不为0
        {
            // io向量指向 http 响应报文 m_write_buf 和 文件地址 m_file_address
            add_headers(m_file_stat.st_size);
            m_iovec[0].iov_base = m_write_buf;
            m_iovec[0].iov_len = m_write_idx;
            m_iovec[1].iov_base = m_file_address;
            m_iovec[1].iov_len = m_file_stat.st_size;
            m_bytes_to_send = m_write_idx + m_file_stat.st_size;
            return true;
        }
        else 
        {
            const char *ok_string = "<html><body></body></html>";
            add_headers(strlen(ok_string));
            if (!add_content(ok_string))
                return false;
        }
    }
    default:
        return false;
    }

    // 请求出错
    // 将 io 向量指向 http 响应报文
    m_iovec[0].iov_base = m_write_buf;
    m_iovec[0].iov_len  = m_write_idx;
    m_iovec_count = 1;
    m_bytes_to_send = m_write_idx;
    return true;
}

void HttpConn::process()
{
    HTTP_CODE read_ret = process_read();
    // NO_REQUEST: 请求不完整，需要继续接收请求数据
    if (read_ret == NO_REQUEST)
    {
        // 重新注册监听事件
        modfd(m_epoll_fd, m_sockfd, EPOLLIN, m_trig_mode);
        return;
    }

    // 完成报文相应
    bool write_ret = process_write(read_ret);
    
    // 失败，断开连接
    if (!write_ret)
        close_conn();
    
    // 注册写事件，... 将 http 相应返回客户端
    modfd(m_epoll_fd, m_sockfd, EPOLLOUT, m_trig_mode);
}