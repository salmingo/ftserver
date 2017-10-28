/*
 * @file FileWritter.cpp 文件写盘管理器定义文件
 * @version 0.1
 * @date 2017-10-28
 */

#include <boost/make_shared.hpp>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "FileWritter.h"
#include "GLog.h"

FileWritter::FileWritter() {
}

FileWritter::~FileWritter() {
	if (thrd_.unique()) {
		thrd_->interrupt();
		thrd_->join();
	}
	quenf_.clear();
}

void FileWritter::UpdateStorage(const string& path) {
	pathRoot_ = path;
}

bool FileWritter::StartService() {
	const CBSlot& slot = boost::bind(&FileWritter::OnNewFile, this, _1, _2);
	RegisterMessage(MSG_NEW_FILE, slot);
	string mqname = "mqfts_file_writter";
	if (!Start(mqname.c_str())) return false;

	return false;
}

void FileWritter::StopService() {

}

void FileWritter::OnNewFile(const long p1, const long p2) {
	FileInfoPtr ptr = quenf_.front();
	string filepath;	// 文件路径
	filepath = pathRoot_ + "/" + ptr->subpath;
	if (access(filepath.c_str(), F_OK) && mkdir(filepath.c_str(), 0755)) {// 检查并创建子目录
		_gLog.Write(LOG_FAULT, "FileWritter::OnNewFile", "failed to create directory<%s>. %s",
				filepath.c_str(), strerror(errno));
	}
	else {// 写入文件
		filepath += "/" + ptr->filename;
		FILE* fp = fopen(filepath.c_str(), "wb");
		if (fp) {
			char* data = ptr->filedata.get();
			int towrite(ptr->filesize), written(0);
			while (written < towrite) {
				written += fwrite(data + written, 1, towrite - written, fp);
			}
			if (fclose(fp)) {
				_gLog.Write(LOG_WARN, "FileWritter::OnNewFile", "some error on saving file<%s>",
						filepath.c_str(), strerror(errno));
			}
			else {
				_gLog.Write("saved file<%s>", filepath.c_str());
				quenf_.pop_front();
			}
		}
		else {
			_gLog.Write(LOG_FAULT, "FileWritter::OnNewFile", "failed to save file<%s>. %s",
					filepath.c_str(), strerror(errno));
		}
	}
}
