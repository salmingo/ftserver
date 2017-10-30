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
#include "handlefs.h"

using namespace boost::posix_time;

TransferAgent::TransferAgent() {

}

TransferAgent::~TransferAgent() {

}

bool TransferAgent::StartService() {
	param_.LoadFile(gConfigPath);

	return true;
}

void TransferAgent::StopService() {

}

int TransferAgent::find_disk() {
	int n = param_.nStorage;
	int t = param_.freeDisk;
	int i, j;

	for (i = param_.iStorage, j = 0; j < n; ++i, ++j) {
		if (i >= n) i -= n;
		if ((disk_capacity(param_.pathStorage[i].c_str()) >> 20) >= t) break;
	}

	return (j == n ? -1 : i);
}

int TransferAgent::free_path(const char* pathname, const int nday) {
	if (nday <= 1) return -2;

	_gLog.Write("try to erase historical data in <%s>", pathname);

	char oldpath[200];
	ptime now(second_clock::local_time()), last;
	ptime::time_duration_type td;
	DIR* dir;
	struct dirent* dirp;
	struct stat st;
	char* name;
	int n(0);

	getcwd(oldpath, 200);
	if (chdir(pathname)) return -3;
	dir = opendir(".");
	while ((dirp = readdir(dir)) != NULL) {
		name = &dirp->d_name[0];
		if (!(strcmp(name, ".") && strcmp(name, ".."))) continue;
		stat(name, &st);
		if (strlen(name) >= 15 && name[0] == 'G' && name[4] == '_') {
			last = from_time_t(st.st_mtim.tv_sec);
			td = now - last;
			if (td.total_seconds() >= (nday * 86400)){
				if (S_ISDIR(st.st_mode)) free_dir(name);
				else remove(name);
				++n;
			}
		}
	}
	closedir(dir);
	chdir(oldpath);

	if (n) _gLog.Write("historical data in <%s> are erased", pathname);
	else _gLog.Write(LOG_WARN, "TransferAgent::free_path", "nothing can be erased");

	return (!n ? -1 : disk_capacity(pathname));
}

bool TransferAgent::free_storage() {
	int now(param_.iStorage), next(param_.iStorage);
	int n(param_.nStorage);
	int limit = param_.freeDisk, size;
	string path;

	do {
		next = (next + 1) % n;
		path = param_.pathStorage[next];
	}while((size = free_path(path.c_str())) < limit && now != next);

	return (size >= limit);
}

long TransferAgent::next_noon() {// 计算当前时钟与之后第一个正午秒数的差距
	ptime now = second_clock::local_time();
	ptime noon(now.date(), hours(12));
	ptime::time_duration_type td = noon - now;
	long secs = td.total_seconds();

	return (secs > 10 ? secs : 86400 + secs);
}

void TransferAgent::thread_transferclient() {
	TransferClientVec::iterator it;
	boost::chrono::seconds period(10);	// 周期: 10秒

	while (1) {
		boost::this_thread::sleep_for(period);

		if (tcli_.size()) {
			mutex_lock lck(mtx_tcli_);
			for (it = tcli_.begin(); it != tcli_.end(); ) {
				if ((*it)->IsOpen()) ++it;
				else it = tcli_.erase(it);
			}
		}
	}
}

void TransferAgent::thread_checkdisk() {
	/**
	 * 工作逻辑与流程:
	 * 1. 12:00 LocalTime启动工作流程
	 * 2. 检查当前使用盘区剩余容量, 若容量小于阈值, 尝试查找其它有足够容量盘区
	 * 3. 若所有盘区容量不足, 尝试删除当前使用盘区的下一个盘区的历史数据
	 * 4. 若删除后仍无可用盘区, 则迭代删除历史数据
	 * 5. 若全部删除后仍无可用盘区, 则触发警报
	 */
	int now, next;
	string path;

	while (1) {
		boost::this_thread::sleep_for(boost::chrono::seconds(next_noon()));
		now = param_.iStorage;
		if ((next = find_disk()) == now) continue; // 当前磁盘空间充足
		if (next >= 0) {// 更换存储盘区
			string path = param_.pathStorage[next];

			_gLog.Write("switching storage path to <%s>", path.c_str());
			param_.iStorage = next;
			if (fwptr_.use_count()) fwptr_->UpdateStorage(path.c_str());
		}
		else {// 删除历史数据
			free_storage();
		}
	}
}
