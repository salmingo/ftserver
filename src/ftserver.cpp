/*!
 * Name        : ftserver.cpp
 * Author      : Xiaomeng Lu
 * Version     : 0.2
 * Copyright   : SVOM Group, NAOC
 * Description : 文件服务器, 接收并存储GWAC相机上传的FITS文件, 并在数据库中注册该文件
 * - 创建网络服务
 * - 接收FITS文件
 * - 存储FITS文件
 * - 注册FITS文件
 * - 记录日志文件
 * - 同步本地时钟
 **/

#include "GLog.h"
#include "parameter.h"
#include "daemon.h"
#include "globaldef.h"
#include "TransferAgent.h"
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;
GLog _gLog(stdout);

long next_noon() {// 计算当前时钟与之后第一个正午秒数的差距
	ptime now = second_clock::local_time();
	ptime noon(now.date(), hours(12));
	ptime::time_duration_type td = noon - now;
	long secs = td.total_seconds();
_gLog.Write("\nNow = %s\nNoon = %s\nsecs = %d",
		to_iso_extended_string(now).c_str(),
		to_iso_extended_string(noon).c_str(),
		secs);
	return (secs > 0 ? secs : 86400 + secs);
}

int main(int argc, char** argv) {
	if (argc >= 2) {
		if (strcmp(argv[1], "-d") == 0) {
//			param_config param;
//			param.InitFile("ftserver.xml");
			_gLog.Write("distance = %d", next_noon());
		}
		else {
			printf("Usage: ftserver <-d>\n");
		}
	}
	else {
		boost::asio::io_service ios;
		boost::asio::signal_set signals(ios, SIGINT, SIGTERM);  // interrupt signal
		signals.async_wait(boost::bind(&boost::asio::io_service::stop, &ios));

		if (!MakeItDaemon(ios)) return 1;
		if (!isProcSingleton(gPIDPath)) {
			_gLog.Write("%s is already running or failed to access PID file", DAEMON_NAME);
			return 2;
		}

		_gLog.Write("Try to launch %s %s %s as daemon", DAEMON_NAME, DAEMON_VERSION, DAEMON_AUTHORITY);
		// 主程序入口
		TransferAgent agent;
		if (agent.StartService()) {
			_gLog.Write("Daemon goes running");
			ios.run();
			agent.StopService();
		}
		else {
			_gLog.Write(LOG_FAULT, NULL, "Fail to launch %s", DAEMON_NAME);
		}
		_gLog.Write("Daemon stopped");
	}

	return 0;
}
