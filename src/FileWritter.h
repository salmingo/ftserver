/*!
 * @file FileWritter.h 文件写盘管理器声明文件
 * @version 0.1
 * @date 2017-10-28
 * @note
 * - 缓存待写盘文件
 * - 创建子目录
 * - 串行写盘
 * - 维护线程
 * - 维护日志
 * - 维护资源
 */

#ifndef FILEWRITTER_H_
#define FILEWRITTER_H_

#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/container/deque.hpp>
#include <string>
#include "MessageQueue.h"

using std::string;

// 数据结构
struct FileInfo {// 待写盘文件信息
	string subpath;		//< 子目录名称
	string filename;		//< 文件名称
	int filesize;		//< 文件大小, 量纲: 字节
	boost::shared_array<char> filedata;	//< 文件内容

public:
	FileInfo(const string& _subpath, const string& _filename, const int _filesize) {
		subpath  = _subpath;
		filename = _filename;
		filesize = _filesize;
		filedata.reset(new char[filesize]);
	}

	FileInfo(const string& _subpath, const string& _filename,
			const int _filesize, boost::shared_array<char> _data) {
		subpath  = _subpath;
		filename = _filename;
		filesize = _filesize;
		filedata = _data;
	}

	virtual ~FileInfo() {
		filedata.reset();
	}
};
typedef boost::shared_ptr<FileInfo> FileInfoPtr;

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
	typedef boost::container::deque<FileInfoPtr> FileInfoQueue;

protected:
	// 成员变量
	string pathRoot_;		//< 当前根路径
	FileInfoQueue quenf_;	//< 文件队列

public:
	// 接口
	/*!
	 * @brief 更新文件存储盘区, 即根路径
	 * @param path 路径名称
	 */
	void UpdateStorage(const string& path);
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

#endif /* FILEWRITTER_H_ */
