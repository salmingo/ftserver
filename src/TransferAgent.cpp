/*
 * @file TransferAgent.cpp 网络文件传输代理定义文件
 * @version 0.1
 * @date 2017-10-29
 */
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "globaldef.h"
#include "GLog.h"
#include "TransferAgent.h"

using namespace boost::posix_time;

TransferAgent::TransferAgent() {
	param_.LoadFile(gConfigPath);
}

TransferAgent::~TransferAgent() {

}

bool TransferAgent::StartService() {

	return true;
}

void TransferAgent::StopService() {

}
