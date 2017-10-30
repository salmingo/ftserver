/*!
 * @file parameter.h 使用XML文件格式管理配置参数
 */

#ifndef PARAMETER_H_
#define PARAMETER_H_

#include <string>
#include <vector>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

using std::string;
using std::vector;

struct param_config {// 软件配置参数
	uint16_t port;	//< 网络服务端口
	bool bNTP;		//< 是否启用NTP
	string ipNTP;	//< NTP主机地址
	int diffNTP;	//< 时钟最大偏差
	bool bDB;		//< 是否启用数据库
	string urlDB;	//< 数据库链接地址
	int nStorage;	//< 文件存储盘区数量
	int iStorage;	//< 当前使用盘区索引
	int iOld;		//< 上次使用盘区索引
	vector<string> pathStorage;	//< 文件存储盘区名称列表
	bool bFreeDisk;	//< 是否启用磁盘清除操作
	int freeDisk;	//< 最小磁盘容量, 量纲: GB. 当小于该值时更换盘区或删除历史数据

private:
	string filepath;	//< 配置文件路径

public:
	virtual ~param_config() {
		if (iStorage != iOld) {
			using boost::property_tree::ptree;

			ptree pt;
			read_xml(filepath, pt, boost::property_tree::xml_parser::trim_whitespace);

			for (ptree::iterator it = pt.begin(); it != pt.end(); ++it) {
				if (boost::iequals((*it).first, "LocalStorage")) {
					(*it).second.put("DiskIndex.<xmlattr>.Value", iStorage);
					break;
				}
			}
			boost::property_tree::xml_writer_settings<std::string> settings(' ', 4);
			write_xml(filepath, pt, std::locale(), settings);
		}
	}
	/*!
	 * @brief 初始化文件filepath, 存储缺省配置参数
	 * @param filepath 文件路径
	 */
	void InitFile(const std::string& filepath) {
		this->filepath = filepath;

		using namespace boost::posix_time;
		using boost::property_tree::ptree;

		ptree pt;

		pathStorage.clear();
		pt.add("version", "0.2");
		pt.add("date", to_iso_string(second_clock::universal_time()));
		pt.add("Server.<xmlattr>.Port",     port = 4020);
		pt.add("NTP.<xmlattr>.Enable",      bNTP = true);
		pt.add("NTP.<xmlattr>.IP",          ipNTP = "172.28.1.3");
		pt.add("NTP.<xmlattr>.Difference",  diffNTP = 5);
		pt.add("Database.<xmlattr>.Enable", bDB = true);
		pt.add("Database.<xmlattr>.URL",    urlDB = "http://172.28.8.8:8080/gwebend/");
		pt.add("CheckDisk.<xmlattr>.Enable", bFreeDisk = false);
		pt.add("CheckDisk.<xmlattr>.MinimumFree", freeDisk = 100);

		ptree& node = pt.add("LocalStorage", "");
		string path = "/data";
		node.add("DiskNumber.<xmlattr>.Value", nStorage = 1);
		node.add("DiskIndex.<xmlattr>.Value",  iStorage = 0);
		node.add("PathRoot#1.<xmlattr>.Name", path);
		pathStorage.push_back(path);
		iOld = iStorage;

		boost::property_tree::xml_writer_settings<std::string> settings(' ', 4);
		write_xml(filepath, pt, std::locale(), settings);
	}

	/*!
	 * @brief 从文件filepath加载配置参数
	 * @param filepath 文件路径
	 */
	void LoadFile(const std::string& filepath) {
		this->filepath = filepath;

		try {
			using boost::property_tree::ptree;

			ptree pt;
			pathStorage.clear();
			read_xml(filepath, pt, boost::property_tree::xml_parser::trim_whitespace);

			BOOST_FOREACH(ptree::value_type const &child, pt.get_child("")) {
				if (boost::iequals(child.first, "Server")) {
					port = child.second.get("Server.<xmlattr>.Port", 4020);
				}
				else if (boost::iequals(child.first, "NTP")) {
					bNTP    = child.second.get("NTP.<xmlattr>.Enable",     true);
					ipNTP   = child.second.get("NTP.<xmlattr>.IP",         "172.28.1.3");
					diffNTP = child.second.get("NTP.<xmlattr>.Difference", 5);
				}
				else if (boost::iequals(child.first, "Database")) {
					bDB   = child.second.get("Database.<xmlattr>.Enable", true);
					urlDB = child.second.get("Database.<xmlattr>.URL",    "http://172.28.8.8:8080/gwebend/");
				}
				else if (boost::iequals(child.first, "CheckDisk")) {
					bFreeDisk= child.second.get("CheckDisk.<xmlattr>.Enable",      false);
					freeDisk = child.second.get("CheckDisk.<xmlattr>.MinimumFree", 100);
				}
				else if (boost::iequals(child.first, "LocalStorage")) {
					boost::format fmt("PathRoot#%d.<xmlattr>.Name");
					nStorage = child.second.get("DiskNumber.<xmlattr>.Value", 1);
					iStorage = child.second.get("DiskIndex.<xmlattr>.Value",  0);
					iOld     = iStorage;
					for (int i = 1; i <= nStorage; ++i) {
						fmt % i;
						string path = child.second.get(fmt.str(), "/data");
						boost::trim_right_copy_if(path, boost::is_punct() || boost::is_space());
						pathStorage.push_back(path);
					}
				}
			}
		}
		catch(boost::property_tree::xml_parser_error& ex) {
			InitFile(filepath);
		}
	}
};

#endif // PARAMETER_H_
