/*!
 * @file FileWritter.h 文件写盘管理器声明文件
 * @version 0.1
 * @date 2017-10-28
 * @note
 * - 缓存待写盘文件
 * - 创建子目录
 * - 串行写盘
 * - 维护日志
 * - 维护资源
 */

#ifndef FILEWRITTER_H_
#define FILEWRITTER_H_

#include <boost/container/deque.hpp>
#include <string.h>
#include <string>
#include "MessageQueue.h"
#include "DataTransfer.h"

using std::string;

// 数据结构
struct FileInfo {// 待写盘文件信息
	string gid;		//< 组标志
	string uid;		//< 单元标志
	string cid;		//< 相机标志
	string grid;		//< 天区划分模式
	string field;	//< 天区编号
	string tmobs;	//< 观测时间
	string subpath;	//< 子目录名称
	string filename;	//< 文件名称
	int filesize;	//< 文件大小, 量纲: 字节
	int rcvsize;		//< 已接收文件大小, 量纲: 字节
	boost::shared_array<char> filedata;	//< 文件内容

public:
	FileInfo(const int _filesize) {
		filesize = _filesize;
		rcvsize  = 0;
		filedata.reset(new char[filesize]);
	}

	/*!
	 * @brief 存储新到达的数据
	 * @param data 新到达数据指针
	 * @param n    新到达数据长度, 量纲: 字节
	 * @return
	 * 文件接收完成
	 */
	bool DataArrive(const char *data, const int n) {
		memcpy(filedata.get() + rcvsize, data, n);
		rcvsize += n;
		return (rcvsize >= filesize);
	}
};
typedef boost::shared_ptr<FileInfo> nfileptr;

class FileWritter : public MessageQueue {
public:
	FileWritter();
	virtual ~FileWritter();

protected:
	// 数据结构
	enum MSG_FW {
		MSG_NEW_FILE = MSG_USER	//< 有新的文件等待写入
	};

	typedef boost::shared_ptr<boost::thread> threadptr;
	typedef boost::unique_lock<boost::mutex> mutex_lock;
	typedef boost::container::deque<nfileptr> nfileQueue;

protected:
	// 成员变量
	string pathRoot_;		//< 当前根路径
	nfileQueue quenf_;	//< 文件队列
	boost::shared_ptr<DataTransfer> db_;	//< 数据库访问接口

public:
	// 接口
	/*!
	 * @brief 启动写盘服务
	 * @return
	 * 服务启动结果
	 */
	bool StartService();
	/*!
	 * @brief 停止写盘服务
	 */
	void StopService();
	/*!
	 * @brief 更新文件存储盘区, 即根路径
	 * @param path 路径名称
	 */
	void UpdateStorage(const char* path);
	/*!
	 * @brief 设置数据库
	 * @param enabled 启用数据库
	 * @param url     URL地址
	 */
	void SetDatabase(bool enabled = false, const char* url = NULL);
	/*!
	 * @brief 尝试保存文件
	 * @param nfptr 待保存文件
	 */
	void SaveFile(nfileptr nfptr);

protected:
	// 功能
	/*!
	 * @brief 响应文件写入磁盘请求
	 * @param p1
	 * @param p2
	 */
	void OnNewFile(const long p1, const long p2);
};
typedef boost::shared_ptr<FileWritter> FileWritePtr;
extern FileWritePtr make_filewritter();

#endif /* FILEWRITTER_H_ */
