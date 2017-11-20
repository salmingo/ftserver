/*
 * @file TransferClient.h 文件传输客户端声明文件, 接收文件
 * @version 0.1
 * @date 2017-10-29
 * @note
 * - 维持网络连接
 * - 接收客户端信息和文件数据
 * - 向客户端反馈状态
 */

#ifndef TRANSFERCLIENT_H_
#define TRANSFERCLIENT_H_

#include <boost/asio.hpp>
#include "MessageQueue.h"
#include "FileWritter.h"

class TransferClient : public MessageQueue {
public:
	TransferClient(FileWritePtr fwptr);
	virtual ~TransferClient();

protected:
	// 数据结构
	typedef boost::shared_ptr<boost::thread> threadptr;

protected:
	// 成员变量
	FileWritePtr fwptr_;		//< 文件写盘接口
	nfileptr fileptr_;		//< 待接收数据

public:
	// 接口

protected:
	// 功能
};
typedef boost::shared_ptr<TransferClient> TransferClientPtr;
/*!
 * @brief 工厂函数, 创建TransferClient指针
 * @param ptr 文件写盘接口
 * @return
 * 文件接收接口
 */
extern TransferClientPtr make_transcli(FileWritePtr ptr);

#endif /* TRANSFERCLIENT_H_ */
