/*
 * @file TransferAgent.h 网络文件传输代理声明文件
 * @version 0.1
 * @date 2017-10-29
 */

#ifndef TRANSFERAGENT_H_
#define TRANSFERAGENT_H_

#include <boost/asio.hpp>
#include <boost/container/stable_vector.hpp>
#include "MessageQueue.h"
#include "TransferClient.h"
#include "FileWritter.h"
#include "parameter.h"
#include "IOServiceKeep.h"

using boost::asio::ip::tcp;

class TransferAgent: public MessageQueue {
public:
	TransferAgent();
	virtual ~TransferAgent();

protected:
	// 数据结构
	typedef boost::container::stable_vector<TransferClientPtr> TransferClientVec;

protected:
	// 成员变量
	param_config param_;		//< 配置参数
	IOServiceKeep keep_;		//< 维持asio::io_service对象
	boost::shared_ptr<tcp::acceptor> acceptor_;	//< 网络服务

	threadptr thrd_tcli_;		//< 线程: 监测文件传输接口有效性
	TransferClientVec tcli_;		//< 文件传输接口集合
	boost::mutex mtx_tcli_;		//< 互斥锁: 文件传输接口集合

	threadptr thrd_chkdsk;		//< 线程: 检查磁盘容量

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
	/*!
	 * @brief 查找符合使用条件的盘区
	 * @return
	 * 可用盘区索引. 若无可用盘区时返回-1, 否则返回其索引
	 */
	int find_disk();
	/*!
	 * @brief 清除制定盘区上的历史数据
	 * @param pathname 路径名称
	 * @param adapter  适配符, 即删除符合适配符文件和目录
	 * @param nday     天数, 即删除早于nday天之前文件和目录
	 * @return
	 * 清除数据后的盘区可用容量
	 */
	int free_path(const char* pathname, const int nday = 7);
	/*!
	 * @brief 清理存储盘区, 直至某一盘区有足够的存储空间
	 * @return
	 * 操作结果
	 */
	bool free_storage();
	/*!
	 * @brief 由当前时间计算下一个正午与当前时间差异的秒数
	 * @return
	 * 秒数
	 * @note
	 * 若当前时钟为上午, 则秒数对应当日正午; 否则对应次日正午
	 */
	long next_noon();
	/*!
	 * @brief 线程: 监测文件传输接口有效性
	 */
	void thread_transferclient();
	/*!
	 * @brief 线程: 检查磁盘容量并删除历史数据
	 * @note
	 * 流程在本地时间12:00左右执行一次
	 */
	void thread_checkdisk();
};

#endif /* TRANSFERAGENT_H_ */
