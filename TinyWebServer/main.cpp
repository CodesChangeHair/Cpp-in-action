#include "config.h"

int main(int argc, char *argv[])
{
	// 需要修改的数据库信息, 登录名，密码，库名
	string user = "wh";
	string password = "The_Beat1es";
	string database_name = "userdb";

	// 命令行解析
	Config config;
	config.parse_arg(argc, argv);

	WebServer server;

	// 初始化
	
	server.init(config.m_port, user, password, database_name, config.m_log_write,
				config.m_opt_linger, config.m_trig_mode, config.m_sql_num, config.m_thread_num,
				config.m_close_log, config.m_actor_mode);

	// 日志
	server.log_write();

	// 数据库
	server.sql_pool();

	// 线程池
	server.thread_pool();

	// 触发模式
	server.trig_mode();

	// 监听
	server.event_listen();

	// 运行
	server.event_loop();

	return 0;
}
