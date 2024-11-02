#pragma once

// #ifdef __cplusplus
// extern "C" {  // C++ 中使用 extern "C" 以确保 C 函数可以被正确链接
// #endif

#define FUSE_USE_VERSION 31

#include <string>
#include <cstring>
#include <fuse3/fuse.h>

// struct options {
// 	std::string filename;
// 	std::string contents;
// 	bool show_help;
// };

struct open_file_info {
	std::string filename;
	int flags;
};

#define OPEN_FILE_LIST_SIZE 100
extern struct open_file_info open_files[OPEN_FILE_LIST_SIZE];
// extern struct options options;

void* fuse_init(struct fuse_conn_info *conn, struct fuse_config *cfg);

int fuse_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);

int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags);

int fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi);

int fuse_open(const char *path, struct fuse_file_info *fi);

int fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int fuse_getxattr(const char* path, const char* name, char* value, size_t size);

int fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int fuse_flush(const char* c, struct fuse_file_info* f);

int fuse_utimens(const char* path,const struct timespec tv[2], struct fuse_file_info *fi);

extern const struct fuse_operations hello_oper;

// #ifdef __cplusplus
// }
// #endif