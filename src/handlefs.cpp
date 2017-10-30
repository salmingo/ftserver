/*!
 * @file handlefs.cpp 文件系统操作定义文件
 * @version 0.1
 * @date 2017-10-27
 */

#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "handlefs.h"

int disk_capacity(const char* pathname) {// 检查盘区可用容量
	struct statfs64 stat;

	if (!statfs64(pathname, &stat))
		return ((stat.f_bsize * stat.f_bavail) >> 10);

	return -1;
}

bool dfs_free_dir(const char* pathname) {// 执行目录或文件删除操作
	DIR* dir;;
	struct dirent* dirp;
	struct stat st;

	if ((dir = opendir(".")) == NULL) return false;
	while((dirp = readdir(dir)) != NULL) {// 遍历删除文件
		if (!(strcmp(dirp->d_name, ".") && strcmp(dirp->d_name, ".."))) continue;
		stat(dirp->d_name, &st);
		if (S_ISDIR(st.st_mode)) {// 迭代删除目录
			chdir(dirp->d_name);
			dfs_free_dir(dirp->d_name);
			chdir("..");
		}
		remove(dirp->d_name);
	}

	closedir(dir);
	return true;
}

bool free_dir(const char* pathname) {// 删除目录或文件
	char oldpath[200];

	getcwd(oldpath, 200);
	if (chdir(pathname)) return false;
	dfs_free_dir(pathname);
	chdir(oldpath);
	remove(pathname);
	return true;
}
