/*!
 * @file AsciiProtocol.h 声明文件, 声明GWAC/GFT系统中字符串型通信协议
 * @version 0.1
 * @date 2017-11-17
 * - 通信协议采用Struct声明
 * - 通信协议继承自ascii_protocol_base
 */

#ifndef ASCIIPROTOCOL_H_
#define ASCIIPROTOCOL_H_

#include <list>
#include <boost/thread.hpp>
#include <float.h>
#include "AsciiProtocolBase.h"

using std::list;

//////////////////////////////////////////////////////////////////////////////
/*--------------------------------- 声明通信协议 ---------------------------------*/

/* GWAC相机辅助程序通信协议: 温度和真空度 */
struct ascii_proto_cooler : public ascii_proto_base {// 温控参数
	float voltage;	//< 工作电压.   量纲: V
	float current;	//< 工作电流.   量纲: A
	float hotend;	//< 热端温度.   量纲: 摄氏度
	float coolget;	//< 探测器温度. 量纲: 摄氏度
	float coolset;	//< 制冷温度.   量纲: 摄氏度

public:
	ascii_proto_cooler() {
		type = "cooler";
		voltage = current = hotend = coolget = coolset = FLT_MIN;
	}
};
typedef boost::shared_ptr<ascii_proto_cooler> apcooler;
extern apcooler make_apcooler();

struct ascii_proto_vacuum : public ascii_proto_base {// 真空度参数
	float voltage;	//< 工作电压.   量纲: V
	float current;	//< 工作电流.   量纲: A
	string pressure;	//< 气压

public:
	ascii_proto_vacuum() {
		type = "vacuum";
		voltage = current = FLT_MIN;
	}
};
typedef boost::shared_ptr<ascii_proto_vacuum> apvacuum;
extern apvacuum make_apvacuum();

/* FITS文件传输 */
struct ascii_proto_fileinfo : public ascii_proto_base {// 文件描述信息, 客户端=>服务器
	string grid;			//< 天区划分模式
	string field;		//< 天区编号
	string tmobs;		//< 观测时间
	string subpath;		//< 子目录名称
	string filename;		//< 文件名称
	int filesize;		//< 文件大小, 量纲: 字节

public:
	ascii_proto_fileinfo() {
		type = "fileinfo";
		filesize = 0;
	}
};
typedef boost::shared_ptr<ascii_proto_fileinfo> apfileinfo;
extern apfileinfo make_apfileinfo();

struct ascii_proto_filestat : public  ascii_proto_base {// 文件传输结果, 服务器=>客户端
	/*!
	 * @member status 文件传输结果
	 * - 1: 服务器完成准备, 通知客户端可以发送文件数据
	 * - 2: 服务器完成接收, 通知客户端可以发送其它文件
	 */
	int status;	//< 文件传输结果

public:
	ascii_proto_filestat() {
		type = "filestat";
		status = 0;
	}
};
typedef boost::shared_ptr<ascii_proto_filestat> apfilestat;
extern apfilestat make_apfilestat();

//////////////////////////////////////////////////////////////////////////////
/*!
 * @class AsciiProtocol 通信协议操作接口, 封装协议解析与构建过程
 */
class AsciiProtocol {
public:
	AsciiProtocol();
	virtual ~AsciiProtocol();

public:
	/* 数据类型 */
	typedef boost::unique_lock<boost::mutex> mutex_lock;	//< 互斥锁
	typedef boost::shared_array<char> charray;	//< 字符数组
	typedef list<string> listring;	//< string列表

protected:
	/* 成员变量 */
	boost::mutex mtx_;	//< 互斥锁
	int ibuf_;			//< 存储区索引
	charray buff_;		//< 存储区

protected:
	/*!
	 * @brief 输出编码后字符串
	 * @param compacted 已编码字符串
	 * @param n         输出字符串长度, 量纲: 字节
	 * @return
	 * 编码后字符串
	 */
	const char* output_compacted(string& output, int& n);
	/*!
	 * @brief 连接关键字和对应数值, 并将键值对加入output末尾
	 * @param output   输出字符串
	 * @param keyword  关键字
	 * @param value    非字符串型数值
	 */
	template <class T1, class T2>
	void join_kv(string& output, T1& keyword, T2& value) {
		boost::format fmt("%1%=%2%,");
		fmt % keyword % value;
		output += fmt.str();
	}
	/*!
	 * @brief 解析关键字和对应数值
	 * @param kv       keyword=value对
	 * @param keyword  关键字
	 * @param value    对应数值
	 * @return
	 * 关键字和数值非空
	 */
	bool resolve_kv(string& kv, string& keyword, string& value);

public:
	/*---------------- 封装通信协议 ----------------*/
	/* GWAC相机辅助程序通信协议: 温度和真空度 */
	/*!
	 * @brief 封装温度协议为字符串
	 * @param proto 协议内容
	 * @param n     封装后字符串长度
	 * @return
	 * 封装后字符串
	 */
	const char *CompactCooler(apcooler proto, int &n);
	/*!
	 * @brief 封装真空度协议为字符串
	 * @param proto 协议内容
	 * @param n     封装后字符串长度
	 * @return
	 * 封装后字符串
	 */
	const char *CompactVacuum(apvacuum proto, int &n);
	/* FITS文件传输 */
	/*!
	 * @brief 封装文件描述信息为字符串
	 * @param proto 协议内容
	 * @param n     封装后字符串长度
	 * @return
	 * 封装后字符串
	 */
	const char *CompactFileInfo(apfileinfo proto, int &n);
	/*!
	 * @brief 封装文件接收结果为字符串
	 * @param proto 协议内容
	 * @param n     封装后字符串长度
	 * @return
	 * 封装后字符串
	 */
	const char *CompactFileStat(apfilestat proto, int &n);

public:
	/*---------------- 解析通信协议 ----------------*/
	/*!
	 * @brief 解析字符串生成结构化通信协议
	 * @param rcvd 待解析字符串
	 * @return
	 * 统一转换为apbase类型
	 */
	apbase Resolve(const char *rcvd);

protected:
	/*---------------- 解析通信协议 ----------------*/
	/*!
	 * @brief 解析字符串为结构化温度协议
	 * @param tokens 协议主体
	 * @return
	 * 转换为apbase的结构化协议
	 */
	apbase resolve_cooler(listring& tokens);
	/*!
	 * @brief 解析字符串为结构化真空度协议
	 * @param tokens 协议主体
	 * @return
	 * 转换为apbase的结构化协议
	 */
	apbase resolve_vacuum(listring& tokens);
	/*!
	 * @brief 解析字符串为结构化文件信息协议
	 * @param tokens 协议主体
	 * @return
	 * 转换为apbase的结构化协议
	 */
	apbase resolve_fileinfo(listring& tokens);
	/*!
	 * @brief 解析字符串为结构化文件接收状态协议
	 * @param tokens 协议主体
	 * @return
	 * 转换为apbase的结构化协议
	 */
	apbase resolve_filestat(listring& tokens);
};

typedef boost::shared_ptr<AsciiProtocol> AscProtoPtr;
extern AscProtoPtr make_ascproto();
//////////////////////////////////////////////////////////////////////////////

#endif /* ASCIIPROTOCOL_H_ */
