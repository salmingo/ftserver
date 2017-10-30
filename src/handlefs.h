/*!
 * @file handlefs.h 文件系统操作声明文件
 * @version 0.1
 * @date 2017-10-27
 * @note
 * 实现对文件系统的基本操作
 * - 检测盘区可用容量
 * - 删除目录或文件
 */

#ifndef HANDLEFS_H_
#define HANDLEFS_H_

/*!
 * @brief 检查磁盘可用容量
 * @param pathname 路径名称
 * @return
 * 磁盘可用容量, 量纲: KB
 */
extern int disk_capacity(const char* pathname);
/*!
 * @brief 执行目录删除操作
 * @param pathname 路径名称
 * @return
 * 操作结果
 * @note
 * free_dir调用dfs_free_dir
 */
extern bool dfs_free_dir(const char* pathname);
/*!
 * @brief 删除指定目录
 * @param pathname 路径名称
 * @return
 * 操作结果
 */
extern bool free_dir(const char* pathname);

#endif
