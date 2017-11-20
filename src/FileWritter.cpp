/*
 * @file FileWritter.cpp 文件写盘管理器定义文件
 * @version 0.1
 * @date 2017-10-28
 */

#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <stdio.h>
#include "FileWritter.h"
#include "GLog.h"

FileWritePtr make_filewritter() {
	return boost::make_shared<FileWritter>();
}

FileWritter::FileWritter() {
}

FileWritter::~FileWritter() {
	StopService();
}

void FileWritter::UpdateStorage(const char* path) {
	pathRoot_ = path;
}

void FileWritter::SetDatabase(bool enabled, const char* url) {
	if (!enabled) db_.reset();
	else if(url) db_.reset(new DataTransfer((char*) url));
}

bool FileWritter::StartService() {
	const CBSlot& slot = boost::bind(&FileWritter::OnNewFile, this, _1, _2);
	RegisterMessage(MSG_NEW_FILE, slot);
	return Start("mqfts_file_writter");
}

void FileWritter::StopService() {
	Stop();
	if (quenf_.size()) {
		_gLog.Write(LOG_WARN, "", "%d unsaved files will be lost", quenf_.size());
		quenf_.clear();
	}
}

void FileWritter::SaveFile(nfileptr nfptr) {
	quenf_.push_back(nfptr);
	PostMessage(MSG_NEW_FILE);
}

void FileWritter::OnNewFile(const long p1, const long p2) {
	namespace fs = boost::filesystem;
	fs::path filepath = pathRoot_;	// 文件路径
	nfileptr ptr = quenf_.front();

	filepath /= ptr->subpath;
	if (!fs::is_directory(filepath) && !fs::create_directory(filepath)) {
		_gLog.Write(LOG_FAULT, "FileWritter::OnNewFile", "failed to create directory<%s>", filepath.c_str());
	}
	else {
		FILE *fp;

		filepath /= ptr->filename;
		if (NULL != (fp = fopen(filepath.c_str(), "wb"))) {
			fwrite(ptr->filedata.get(), 1, ptr->filesize, fp);
			fclose(fp);

			if (db_.unique()) {
				char status[200];
				db_->regOrigImage(ptr->gid.c_str(), ptr->uid.c_str(), ptr->cid.c_str(),
						ptr->grid.c_str(), ptr->field.c_str(), ptr->filename.c_str(),
						filepath.c_str(), ptr->tmobs.c_str(), status);
			}
			quenf_.pop_front();

			_gLog.Write("Received: %s", filepath.c_str());
		}
		else {
			_gLog.Write(LOG_FAULT, "FileWritter::OnNewFile", "failed to create file<%s>. %s",
					filepath.c_str(), strerror(errno));
		}
	}
}
