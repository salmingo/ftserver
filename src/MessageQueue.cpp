/*
 * @file MessageQueue.cpp 定义文件, 基于boost::interprocess::ipc::message_queue封装消息队列
 * @version 0.2
 * @date 2017-10-02
 */

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include "MessageQueue.h"
#include "GLog.h"

#define MQFUNC_SIZE		1024

MessageQueue::MessageQueue() {
	funcs_.reset(new CallbackFunc[MQFUNC_SIZE]);
}

MessageQueue::~MessageQueue() {
	Stop();
}

bool MessageQueue::RegisterMessage(const long id, const CBSlot& slot) {
	long pos(id - MSG_USER);
	bool rslt = pos >= 0 && pos < MQFUNC_SIZE;
	if (rslt) {
		funcs_[pos].connect(slot);
	}
	return rslt;
}

void MessageQueue::PostMessage(const long id, const long p1, const long p2) {
	if (mq_.unique()) {
		MSG_UNIT msg(id, p1, p2);
		mq_->send(&msg, sizeof(MSG_UNIT), 1);
	}
}

void MessageQueue::SendMessage(const long id, const long p1, const long p2) {
	if (mq_.unique()) {
		MSG_UNIT msg(id, p1, p2);
		mq_->send(&msg, sizeof(MSG_UNIT), 10);
	}
}

bool MessageQueue::Start(const char* name) {
	if (thrd_.unique()) return true;

	try {
		mqname_ = name;
		message_queue::remove(name);
		mq_.reset(new message_queue(boost::interprocess::create_only, name, 1024, sizeof(MSG_UNIT)));
		thrd_.reset(new boost::thread(boost::bind(&MessageQueue::thread_body, this)));

		return true;
	}
	catch(boost::interprocess::interprocess_exception& ex) {
		_gLog.Write(LOG_FAULT, "MessageQueue::Start", "%s", ex.what());
		return false;
	}
}

void MessageQueue::Stop() {
	if (thrd_.unique()) {
		SendMessage(MSG_QUIT);
		thrd_->join();
	}
	if (mq_.unique()) {
		message_queue::remove(mqname_.c_str());
	}
}

void MessageQueue::thread_body() {
	MSG_UNIT msg;
	message_queue::size_type szrcv;
	message_queue::size_type szmsg = sizeof(MSG_UNIT);
	uint32_t priority;
	long pos;

	do {
		mq_->receive(&msg, szmsg, szrcv, priority);
		if ((pos = msg.id - MSG_USER) >= 0 && pos < MQFUNC_SIZE) {// 检测消息代码有效性
			if (funcs_[pos].empty()) {// 检测回调函数有效性
				_gLog.Write(LOG_WARN, "MessageQueue::thread_body", "Undefined callback function for message id<%d>", msg.id);
			}
			else {// 执行回调函数
				(funcs_[pos])(msg.par1, msg.par2);
			}
		}
		else {
			_gLog.Write(LOG_FAULT, "MessageQueue::thread_body", "Undefined message id<%d>", msg.id);
		}
	} while(msg.id != MSG_QUIT);
}
