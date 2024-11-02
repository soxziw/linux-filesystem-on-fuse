//system calls for phase 1:
//setuid
//setgid
//open
//write
//mkdir
//cd
//ls
//stat
//opendir
//readdir
//closedir

//system calls for phase 2:


/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 */

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include "fuse_impl.h"

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */

struct open_file_info open_files[OPEN_FILE_LIST_SIZE];
struct options options;

void* fuse_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

int fuse_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	} else if (strcmp(path+1, options.filename.c_str()) == 0) {
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = options.contents.size();
	} else
		res = -ENOENT;

	return res;
}

int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags)
{
    (void) offset;
    (void) fi;

    // Ensure the path is the root directory
    if (strcmp(path, "/") == 0)
        return -ENOENT;
;
    // Prepare stat structure for directory entries
    struct stat st;
    memset(&st, 0, sizeof(st));

    // Add "." and ".." entries
    st.st_mode = S_IFDIR | 0755;
    filler(buf, ".", &st, 0, static_cast<fuse_fill_dir_flags>(0));
    filler(buf, "..", &st, 0, static_cast<fuse_fill_dir_flags>(0));

    // Add the "hello" file entry
    st.st_mode = S_IFREG | 0777;
    st.st_nlink = 1;
    st.st_size = options.contents.size();
    filler(buf, options.filename.c_str(), &st, 0, static_cast<fuse_fill_dir_flags>(0));

    return 0;
}
int fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
	int i = 0;
	for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
		if (!open_files[i].filename.empty() && open_files[i].filename == path) { //file is already opened
			return i;
		}	
	}
	for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
		if (open_files[i].filename.empty()) {
			open_files[i].filename = path;
			return i;
		}	
	}
	return -1;
}

int fuse_open(const char *path, struct fuse_file_info *fi) {
	int i = 0;
	for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
		if (!open_files[i].filename.empty() && open_files[i].filename == path) { //file is already opened
			return i;
		}	
	}
	for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
		if (open_files[i].filename.empty()) {
			open_files[i].filename = path;
			return i;
		}	
	}
	return -1;
}

int fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	size_t len;
	(void) fi;
	if(strcmp(path+1, options.filename.c_str()) != 0)
		return -ENOENT;

	len = options.contents.size();
	if (offset < (off_t)len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, options.contents.c_str() + offset, size);
	} else
		size = 0;

	return size;
}

int fuse_getxattr(const char* path, const char* name, char* value, size_t size) {
	return 0;
}

static void send_mail(char* buf) {
	std::string command = buf;
	command = "echo \"" + command + "\" | mail -s 'CS270 testing' jiamingxu@ucsb.edu";
	FILE *fp = popen(command.c_str(), "w");
	pclose(fp);
	return;
}

int fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	// check if file to be written to is opened
	short is_open = 0;
	for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
		if (!open_files[i].filename.empty() && open_files[i].filename == path) {
			is_open = 1;
			break;
		}
	}
	if (!is_open) return -EBADF;

	char message[size];
	strcpy(message, buf);
	send_mail(message);
	return sizeof(message);
}

int fuse_flush(const char* c, struct fuse_file_info* f) {
	return 0;
}

const struct fuse_operations hello_oper = {
    .getattr    = fuse_getattr,
    .open       = fuse_open,
    .read       = fuse_read,
    .write      = fuse_write,
    .flush      = fuse_flush,
    .getxattr   = fuse_getxattr,
    .readdir    = fuse_readdir,
    .init       = fuse_init,
    .create     = fuse_create,
};
