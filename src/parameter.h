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
	vector<string> pathStorage;	//< 文件存储盘区名称列表

public:
	/*!
	 * @brief 初始化文件filepath, 存储缺省配置参数
	 * @param filepath 文件路径
	 */
	void InitFile(const std::string& filepath) {
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

		ptree& node = pt.add("LocalStorage", "");
		string path = "/data";
		node.add("Number", nStorage = 1);
		node.add("PathRoot#1", path);
		pathStorage.push_back(path);

		boost::property_tree::xml_writer_settings<std::string> settings(' ', 4);
		write_xml(filepath, pt, std::locale(), settings);
	}

	/*!
	 * @brief 从文件filepath加载配置参数
	 * @param filepath 文件路径
	 */
	void LoadFile(const std::string& filepath) {
		try {
			using boost::property_tree::ptree;

			std::string value;
			ptree pt;
			pathStorage.clear();
			read_xml(filepath, pt, boost::property_tree::xml_parser::trim_whitespace);

			port    = pt.get("Server.<xmlattr>.Port",     4020);
			bNTP    = pt.get("NTP.<xmlattr>.Enable",      true);
			ipNTP   = pt.get("NTP.<xmlattr>.IP",          "172.28.1.3");
			diffNTP = pt.get("NTP.<xmlattr>.Difference",  5);
			bDB     = pt.get("Database.<xmlattr>.Enable", true);
			urlDB   = pt.get("Database.<xmlattr>.URL",    "http://172.28.8.8:8080/gwebend/");

			BOOST_FOREACH(ptree::value_type const &child, pt.get_child("")) {
				if (boost::iequals(child.first, "LocalStorage")) {
					boost::format fmt("PathRoot#%d");
					nStorage = child.second.get("Number", 1);
					for (int i = 1; i <= nStorage; ++i) {
						fmt % i;
						string path = child.second.get(fmt.str(), "/data");
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
