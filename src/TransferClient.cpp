/*
 * @file TransferClient.cpp 文件传输客户端定义文件, 接收网络文件
 * @version 0.1
 * @date 2017-10-29
 */

#include "GLog.h"
#include "TransferClient.h"

TransferClientPtr make_transcli(FileWritePtr ptr) {
	return boost::make_shared<TransferClient>(ptr);
}

TransferClient::TransferClient(FileWritePtr fwptr) {
	fwptr_ = fwptr;
}

TransferClient::~TransferClient() {

}
