CXX ?= g++				# 定义 C++ 编译期 g++

DEBUG ?= 1				# 定义调试模式开关，默认为开
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

# 主编译目标：生成名为 server 的可执行文件
server: main.cpp  ./timer/lst_timer.cpp ./http/http_conn.cpp ./log/log.cpp ./CGImysql/sql_connection_pool.cpp  webserver.cpp config.cpp
	$(CXX) -o server  $^ $(CXXFLAGS) -lpthread -lmysqlclient
    # $^ 表示所有依赖文件
    # -lpthread   : 链接 pthread 线程库
    # -lmysqlclient : 链接 MySQL 客户端库

clean:
	rm  -r server	# 删除生成的 server 可执行文件