#pragma once

#include <stddef.h>
#include <string.h>

#include "config.h"


#define MAX_KEY_LEN	128
#define MAX_VALUE_LEN	512

#if (ENABLE_LOG != 0)

#define LOG(_fmt, ...) fprintf(stdout, "[%s:%d]: " _fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#else

#define LOG(_fmt, ...)

#endif 

typedef int(*CALLBACK)(int fd, int epoll_fd);

// 存储每个连接的buffer, 以保存多次连接的数据
struct conn_item {
    int fd;

    char rbuffer[BUFFER_SIZE];  // 读缓冲区
    int  ridx;

    char wbuffer[BUFFER_SIZE];  // 写缓冲区
    int  widx;

    // char resource[BUFFER_SIZE];  // 资源 例如网页html

    // input事件: 客户建立连接请求 or 客户输入请求 处理
    CALLBACK recv_callback;  // recv_callback = accept_cb / recv_cb
    
    CALLBACK send_callback;
};

#if ENABLE_HTTP_RESPONSE
typedef struct conn_item connection_t;
#endif


int kvstore_request(connection_t* conn);

void *kvstore_malloc(size_t size);
void kvstore_free(void *ptr);



#if (ENABLE_NETWORK_SELECT == NETWORK_EPOLL)
int epoll_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_NTYCO)
int ntyco_entry();
#endif 

#if (ENABLE_KVSENGINE_SELECT == ENABLE_ARRAY_KVSENGINE)

int init_array_kvengine();
void destroy_array_kvengine();
int kvstore_array_set(char *key, char *value);
char *kvstore_array_get(char *key);
int kvstore_array_modify(char *key, char *value);
int kvstore_array_delete(char* key);
int kvstore_array_count(void);

#elif (ENABLE_KVSENGINE_SELECT == ENABLE_RBTREE_KVSENGINE)

int  init_rbtree_kvengine();
void destroy_rbtree_kvengine();
int kvstore_rbtree_set(char *key, char *value);
char *kvstore_rbtree_get(char *key);
int kvstore_rbtree_modify(char *key, char *value);
int kvstore_rbtree_delete(char* key);
int kvstore_rbtree_count(void);

#elif (ENABLE_KVSENGINE_SELECT == ENABLE_HASH_KVSENGINE)

int  init_hashtable_kvengine();
void destroy_hashtable_kvengine();
int kvstore_hashtable_set(char *key, char *value);
char *kvstore_hashtable_get(char *key);
int kvstore_hashtable_modify(char *key, char *value);
int kvstore_hashtable_delete(char* key);
int kvstore_hashtable_count(void);

#endif 

