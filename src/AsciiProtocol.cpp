/*!
 * @file AsciiProtocol.cpp 定义文件, 定义GWAC/GFT系统中字符串型通信协议相关操作
 * @version 0.1
 * @date 2017-11-17
 **/

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <stdlib.h>
#include "AsciiProtocol.h"

using namespace boost;

//////////////////////////////////////////////////////////////////////////////
/* 为各协议生成共享型指针 */

/* GWAC相机辅助程序通信协议: 温度和真空度 */
apcooler make_apcooler() {
	return boost::make_shared<ascii_proto_cooler>();
}

apvacuum make_apvacuum() {
	return boost::make_shared<ascii_proto_vacuum>();
}

/* FITS文件传输 */
apfileinfo make_apfileinfo() {
	return boost::make_shared<ascii_proto_fileinfo>();
}

apfilestat make_apfilestat() {
	return boost::make_shared<ascii_proto_filestat>();
}

AscProtoPtr make_ascproto() {
	return boost::make_shared<AsciiProtocol>();
}
//////////////////////////////////////////////////////////////////////////////

AsciiProtocol::AsciiProtocol() {
	ibuf_ = 0;
	buff_.reset(new char[1024 * 10]); //< 存储区
}

AsciiProtocol::~AsciiProtocol() {
}

const char* AsciiProtocol::output_compacted(string& output, int& n) {
	mutex_lock lck(mtx_);
	char* buff = buff_.get() + ibuf_ * 1024;
	if (++ibuf_ == 10) ibuf_ = 0;
	trim_right_if(output, is_punct() || is_space());
	n = sprintf(buff, "%s\n", output.c_str());
	return buff;
}

bool AsciiProtocol::resolve_kv(string& kv, string& keyword, string& value) {
	char seps[] = "=";	// 分隔符: 等号
	listring tokens;

	keyword = "";
	value   = "";
	algorithm::split(tokens, kv, is_any_of(seps), token_compress_on);
	if (!tokens.empty()) { keyword = tokens.front(); trim(keyword); tokens.pop_front(); }
	if (!tokens.empty()) { value   = tokens.front(); trim(value); }
	return (!(keyword.empty() || value.empty()));
}

//////////////////////////////////////////////////////////////////////////////
/*---------------- 封装通信协议 ----------------*/
const char *AsciiProtocol::CompactCooler(apcooler proto, int &n) {
	if (!proto.use_count()) return NULL;

	string output = proto->type + " ";

	if (!proto->utc.empty()) join_kv(output, "time", proto->utc);
	join_kv(output, "group_id", proto->gid);
	join_kv(output, "unit_id",  proto->uid);
	join_kv(output, "cam_id",   proto->cid);
	join_kv(output, "voltage",  proto->voltage);
	join_kv(output, "current",  proto->current);
	join_kv(output, "hotend",   proto->hotend);
	join_kv(output, "coolget",  proto->coolget);
	join_kv(output, "coolset",  proto->coolset);

	return output_compacted(output, n);
}

const char *AsciiProtocol::CompactVacuum(apvacuum proto, int &n) {
	if (!proto.use_count()) return NULL;

	string output = proto->type + " ";

	if (!proto->utc.empty()) join_kv(output, "time", proto->utc);
	join_kv(output, "group_id", proto->gid);
	join_kv(output, "unit_id",  proto->uid);
	join_kv(output, "cam_id",   proto->cid);
	join_kv(output, "voltage",  proto->voltage);
	join_kv(output, "current",  proto->current);
	join_kv(output, "pressure", proto->pressure);

	return output_compacted(output, n);
}

const char *AsciiProtocol::CompactFileInfo(apfileinfo proto, int &n) {
	if (!proto.use_count()) return NULL;

	string output = proto->type + " ";
	join_kv(output, "group_id", proto->gid);
	join_kv(output, "unit_id",  proto->uid);
	join_kv(output, "cam_id",   proto->cid);
	join_kv(output, "grid_id",  proto->grid);
	join_kv(output, "field_id", proto->field);
	join_kv(output, "tmobs",    proto->tmobs);
	join_kv(output, "subpath",  proto->subpath);
	join_kv(output, "filename", proto->filename);
	join_kv(output, "filesize", proto->filesize);

	return output_compacted(output, n);
}

const char *AsciiProtocol::CompactFileStat(apfilestat proto, int &n) {
	if (!proto.use_count()) return NULL;

	string output = proto->type + " ";
	join_kv(output, "status", proto->status);
	return output_compacted(output, n);
}

//////////////////////////////////////////////////////////////////////////////
apbase AsciiProtocol::Resolve(const char *rcvd) {
	const char seps[] = ",", *ptr;
	char ch;
	listring tokens;
	apbase proto;
	string type;

	for (ptr = rcvd; *ptr && *ptr != ' '; ++ptr) type += *ptr;
	while (*ptr && *ptr == ' ') ++ptr;

	if (*ptr) algorithm::split(tokens, ptr, is_any_of(seps), token_compress_on);
	if ((ch = type[0]) == 'a') {

	}
	else if (ch == 'b') {

	}
	else if (ch == 'c') {
		if (iequals(type, "cooler")) proto = resolve_cooler(tokens);
	}
	else if (ch == 'f') {
		if      (iequals(type, "fileinfo")) proto = resolve_fileinfo(tokens);
		else if (iequals(type, "filestat")) proto = resolve_filestat(tokens);
	}
	else if (ch == 'v') {
		if (iequals(type, "vacuum")) proto = resolve_vacuum(tokens);
	}

	return proto;
}

apbase AsciiProtocol::resolve_cooler(listring& tokens) {
	apcooler proto = make_apcooler();
	listring::iterator itend = tokens.end();
	string keyword, value;

	for (listring::iterator it = tokens.begin(); it != itend; ++it) {// 遍历键值对
		if (!resolve_kv(*it, keyword, value)) continue;
		// 识别关键字
		if      (iequals(keyword, "group_id")) proto->gid = value;
		else if (iequals(keyword, "unit_id"))  proto->uid = value;
		else if (iequals(keyword, "cam_id"))   proto->cid = value;
		else if (iequals(keyword, "voltage"))  proto->voltage = atof(value.c_str());
		else if (iequals(keyword, "current"))  proto->current = atof(value.c_str());
		else if (iequals(keyword, "hotend"))   proto->hotend  = atof(value.c_str());
		else if (iequals(keyword, "coolget"))  proto->coolget = atof(value.c_str());
		else if (iequals(keyword, "coolset"))  proto->coolset = atof(value.c_str());
		else if (iequals(keyword, "time"))     proto->utc = value;
	}

	return to_apbase(proto);
}

apbase AsciiProtocol::resolve_vacuum(listring& tokens) {
	apvacuum proto = make_apvacuum();
	listring::iterator itend = tokens.end();
	string keyword, value;

	for (listring::iterator it = tokens.begin(); it != itend; ++it) {// 遍历键值对
		if (!resolve_kv(*it, keyword, value)) continue;
		// 识别关键字
		if      (iequals(keyword, "group_id")) proto->gid = value;
		else if (iequals(keyword, "unit_id"))  proto->uid = value;
		else if (iequals(keyword, "cam_id"))   proto->cid = value;
		else if (iequals(keyword, "voltage"))  proto->voltage = atof(value.c_str());
		else if (iequals(keyword, "current"))  proto->current = atof(value.c_str());
		else if (iequals(keyword, "pressure")) proto->pressure = value;
		else if (iequals(keyword, "time"))     proto->utc = value;
	}

	return to_apbase(proto);
}

apbase AsciiProtocol::resolve_fileinfo(listring& tokens) {
	apfileinfo proto = make_apfileinfo();
	listring::iterator itend = tokens.end();
	string keyword, value;

	for (listring::iterator it = tokens.begin(); it != itend; ++it) {// 遍历键值对
		if (!resolve_kv(*it, keyword, value)) continue;
		// 识别关键字
		if      (iequals(keyword, "group_id")) proto->gid      = value;
		else if (iequals(keyword, "unit_id"))  proto->uid      = value;
		else if (iequals(keyword, "cam_id"))   proto->cid      = value;
		else if (iequals(keyword, "grid_id"))  proto->grid     = value;
		else if (iequals(keyword, "field_id")) proto->field    = value;
		else if (iequals(keyword, "tmobs"))    proto->tmobs    = value;
		else if (iequals(keyword, "subpath"))  proto->subpath  = value;
		else if (iequals(keyword, "filename")) proto->filename = value;
		else if (iequals(keyword, "filesize")) proto->filesize = atoi(value.c_str());
	}

	return to_apbase(proto);
}

apbase AsciiProtocol::resolve_filestat(listring& tokens) {
	apfilestat proto = make_apfilestat();
	listring::iterator itend = tokens.end();
	string keyword, value;

	for (listring::iterator it = tokens.begin(); it != itend; ++it) {// 遍历键值对
		if (!resolve_kv(*it, keyword, value)) continue;
		// 识别关键字
		if (iequals(keyword, "status")) proto->status = atoi(value.c_str());
	}

	return to_apbase(proto);
}
