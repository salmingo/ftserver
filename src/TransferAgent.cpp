/*
 * @file TransferAgent.cpp 网络文件传输代理定义文件
 * @version 0.1
 * @date 2017-10-29
 */
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
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
	/* 创建文件存储接口 */
	fwptr_ = make_filewritter();
	fwptr_->SetDatabase(param_.bDB, param_.urlDB.c_str());
	/* 查找可用磁盘存储空间 */
	const char *path = find_storage();
	if (path) {
		fwptr_->UpdateStorage(path);
		if (fwptr_->StartService()) {
			_gLog.Write(LOG_FAULT, NULL, "failed to start file writter service");
			return false;
		}
	}
	else {
		_gLog.Write(LOG_FAULT, NULL, "no disk has enough free capacity for raw data");
		return false;
	}
	/* 创建消息队列 */
	string mqname = "msgque_ftserver";
	register_messages();
	if (!Start(mqname.c_str())) {
		_gLog.Write(LOG_FAULT, NULL, "failed to create message queue<%s>", mqname.c_str());
		return false;
	}
	/* 启动服务器 */
	const TCPServer::CBSlot &slot = boost::bind(&TransferAgent::network_accept, this, _1, _2);
	tcpsrv_ = maketcp_server();
	tcpsrv_->RegisterAccespt(slot);
	if (!tcpsrv_->CreateServer(param_.port)) {
		_gLog.Write(LOG_FAULT, NULL, "failed to create server on port<%d>", param_.port);
		return false;
	}
	/* 启动时钟同步 */
	if (param_.bNTP) {
		ntp_ = make_ntp(param_.ipNTP.c_str(), 123, param_.diffNTP);
		ntp_->EnableAutoSynch();
	}
	/* 启动线程 */
	thrdIdle_.reset(new boost::thread(boost::bind(&TransferAgent::thread_idle, this)));
	thrdAutoFree_.reset(new boost::thread(boost::bind(&TransferAgent::thread_autofree, this)));

	return true;
}

void TransferAgent::StopService() {
	interrupt_thread(thrdIdle_);
	interrupt_thread(thrdAutoFree_);
	if (thrdFreeStorage_) thrdFreeStorage_->interrupt();
	if (thrdFreeTemplate_) thrdFreeTemplate_->interrupt();
	filercv_.clear();
	Stop();
}

void TransferAgent::register_messages() {
	const CBSlot &slot1 = boost::bind(&TransferAgent::on_clean_storage,  this, _1, _2);
	const CBSlot &slot2 = boost::bind(&TransferAgent::on_clean_template, this, _1, _2);

	RegisterMessage(MSG_CLEAN_STORAGE,  slot1);
	RegisterMessage(MSG_CLEAN_TEMPLATE, slot2);
}

void TransferAgent::network_accept(const TcpCPtr&client, const long server) {
	mutex_lock lck(mtx_filercv_);
	FileRcvPtr receiver = make_filercv(fwptr_);
	if (receiver->CoupleNetwork(client)) filercv_.push_back(receiver);
}

const char *TransferAgent::find_storage() {
	namespace fs = boost::filesystem;
	int n = param_.nStorage, iNow = param_.iStorage, iNew, i;
	fs::path path;
	fs::space_info space;
	for (i = 0, iNew = iNow; i < n; ++i, ++iNew) {
		if (iNew >= n) iNew -= n;
		path = param_.pathStorage[iNew];
		space = fs::space(path);
		if ((space.capacity >> 30) >= param_.minDiskStorage) break;
	}
	if (i < n && iNow != iNew) param_.iStorage = iNew;
	return i == n ? NULL : param_.pathStorage[iNew].c_str();
}

void TransferAgent::thread_free_storage() {
	int iNow = param_.iStorage;
	const char *path = find_storage();
	if (path) {// 更新存储空间
		if (iNow != param_.iStorage) fwptr_->UpdateStorage(path);
	}
	else {// 启动清理流程
		if (++iNow >= param_.nStorage) iNow -= param_.nStorage;
		namespace fs = boost::filesystem;
		fs::path path(param_.pathStorage[iNow]); // 清除该路径下所有文件
	}
}

void TransferAgent::thread_free_template() {

}

void TransferAgent::thread_idle() {
	boost::chrono::minutes period(1);
	FileRcvVec::iterator it;

	while(1) {
		boost::this_thread::sleep_for(period);

		mutex_lock lck(mtx_filercv_);
		for (it = filercv_.begin(); it != filercv_.end(); ) {
			if ((*it)->IsAlive()) ++it;
			else it = filercv_.erase(it);
		}
	}
}

void TransferAgent::thread_autofree() {
	while(1) {
		boost::this_thread::sleep_for(boost::chrono::seconds(next_noon()));
		PostMessage(MSG_CLEAN_STORAGE);
		PostMessage(MSG_CLEAN_TEMPLATE);
	}
}

long TransferAgent::next_noon() {
	ptime now(second_clock::local_time());
	ptime noon(now.date(), hours(12));
	long secs = (noon - now).total_seconds();
	return secs < 10 ? secs + 86400 : secs;
}

void TransferAgent::on_clean_storage(long param1, long param2) {
	thrdFreeStorage_.reset(new boost::thread(boost::bind(&TransferAgent::thread_free_storage, this)));
	thrdFreeStorage_->join();
	thrdFreeStorage_.reset();
}

void TransferAgent::on_clean_template(long param1, long param2) {
	thrdFreeTemplate_.reset(new boost::thread(boost::bind(&TransferAgent::thread_free_template, this)));
	thrdFreeTemplate_->join();
	thrdFreeTemplate_.reset();
}
