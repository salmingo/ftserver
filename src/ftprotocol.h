/*!
 * @file ftsprotocol.h 文件服务器通信协议
 * @version 0.1
 * @date 2017-10-29
 * @note
 * - 声明GWAC系统FITS文件传输通信协议
 * - 文件传输流程
 *  1. 客户端发送FileInfo信息
 *  2. 服务器回应Status=1, 表明完成准备, 可以接收文件数据
 *  3. 客户端发送文件数据
 *  4. 服务器回应Status=2, 表明完成文件接收
 * - 约束与定义
 *  1. FileInfo和Status协议采用编码字符串, 以换行符结束, 单元间以空格分割, 单元内以等号分割
 *  2. 文件数据以TCP协议发送, 不做容错处理
 * - 协议编码格式:
 *  1. FileInfo
 * FileInfo gid=<>, uid=<>, cid=<>, grid=<>, field=<>, tmobs=<>, subpath=<>, filename=<>, filesize=<><LN>
 *  2. Status
 * Status code=<><LN>
 */

#ifndef FTPROTOCOL_H_
#define FTPROTOCOL_H_

#include <string>
#include <boost/smart_ptr.hpp>

using std::string;

// 数据结构
struct ft_fileinfo {// 待写盘文件信息
	string gid;		//< 组标志
	string uid;		//< 单元标志
	string cid;		//< 相机标志
	string grid;			//< 天区划分模式
	string field;		//< 天区编号
	string tmobs;		//< 观测时间
	string subpath;		//< 子目录名称
	string filename;		//< 文件名称
	int filesize;		//< 文件大小, 量纲: 字节
	boost::shared_array<char> filedata;	//< 文件内容

public:
	ft_fileinfo(const int _filesize) {
		filesize = _filesize;
		filedata.reset(new char[filesize]);
	}
};
typedef boost::shared_ptr<ft_fileinfo> nfileptr;

#endif
