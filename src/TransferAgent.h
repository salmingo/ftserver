/*!
 * @file TransferAgent.h 网络文件传输代理声明文件
 * @version 0.1
 * @date 2017-10-29
 * @note
 * - 处理网络连接, 为其创建对象TransferClient
 * - 维持线程, 检查、切换与清除原始数据磁盘空间
 * - 维持线程, 检查与清除模板数据磁盘空间
 */

#ifndef TRANSFERAGENT_H_
#define TRANSFERAGENT_H_

#include <boost/container/stable_vector.hpp>
#include "TransferClient.h"
#include "FileWritter.h"
#include "parameter.h"
#include "tcpasio.h"

class TransferAgent {
public:
	TransferAgent();
	virtual ~TransferAgent();

public:
	// 数据结构
	typedef boost::shared_ptr<boost::thread> threadptr;
	typedef boost::unique_lock<boost::mutex> mutex_lock;
	typedef boost::container::stable_vector<TransferClientPtr> TransferClientVec;

protected:
	// 成员变量
	param_config param_;		//< 配置参数
	FileWritePtr fwptr_;		//< 文件写盘接口

public:
	// 接口
	/*!
	 * @brief 尝试启动服务
	 * @return
	 * 服务启动结果
	 */
	bool StartService();
	/*!
	 * @brief 停止服务
	 */
	void StopService();

protected:
	// 功能
};

#endif /* TRANSFERAGENT_H_ */
