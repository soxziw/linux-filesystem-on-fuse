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
// struct options options;

open_file_info* find_path(const char* path){
  for (int count = 0; count < OPEN_FILE_LIST_SIZE; count++) {
        if (!open_files[count].filename.empty() && open_files[count].filename == path)
            return &open_files[count];
    }
	return NULL;
}
// #define OPTION(t, p)                           \
//     { t, offsetof(struct options, p), 1 }
// const struct fuse_opt option_spec[] = {
// 	OPTION("--name=%s", filename),
// 	OPTION("--contents=%s", contents),
// 	OPTION("-h", show_help),
// 	OPTION("--help", show_help),
// 	FUSE_OPT_END
// };

void* fuse_init(struct fuse_conn_info* conn,
			struct fuse_config* cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

int fuse_getattr(const char* path, struct stat* stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	} else{
        struct open_file_info *file = find_path(path);
        if (file == NULL) {
            res = -ENOENT;
        } else {
            stbuf->st_mode = S_IFREG | 0777;
            stbuf->st_nlink = 1;
            // Optionally, set st_size to a fixed value or 0 since you're not storing content
            stbuf->st_size = 0;
            // Optionally, set timestamps
            stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);
        }
    }
	return res;
}

int fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info* fi,
                         enum fuse_readdir_flags flags)
{
    (void) offset;
    (void) fi;

    // Ensure the path is the root directory
    if (strcmp(path, "/") != 0)
        return -ENOENT;

    // Prepare stat structure for directory entries
    struct stat st;
    memset(&st, 0, sizeof(st));

    // Add "." and ".." entries
    st.st_mode = S_IFDIR | 0755;
    filler(buf, ".", &st, 0, static_cast<fuse_fill_dir_flags>(0));
    filler(buf, "..", &st, 0, static_cast<fuse_fill_dir_flags>(0));
	
	for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        if (!open_files[i].filename.empty()) {
            // Remove the leading '/' from the filename
            std::string name = open_files[i].filename;
            if (name[0] == '/')
                name += 1;
            filler(buf, name.c_str(), &st, 0, static_cast<fuse_fill_dir_flags>(0));
        }
    }


    return 0;
}

int fuse_open(const char* path, struct fuse_file_info* fi) {
    open_file_info* paths=find_path(path);
	if (paths == NULL)return -ENOENT;

    return 0;
}

// int fuse_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
// 	size_t len;
// 	(void) fi;
// 	if(strcmp(path+1, options.filename.c_str()) != 0)
// 		return -ENOENT;

// 	len = options.contents.size();
// 	if (offset < len) {
// 		if (offset + size > len)
// 			size = len - offset;
// 		memcpy(buf, options.contents.c_str() + offset, size);
// 	} else
// 		size = 0;

// 	return size;
// }

int fuse_getxattr(const char* path, const char* name, char* value, size_t size) {
	return 0;
}

void send_mail(char* buf) {
	//get command length to malloc
	int command_length = 1 + strlen("echo \"") + strlen(buf) + strlen("\" | mail -s 'CS270 testing' brandon_lee@ucsb.edu");
	char* command = (char*)malloc(command_length * sizeof(char));
	strcpy(command, "echo \"");
	strcat(command, buf);
	strcat(command, "\" | mail -s 'CS270 testing' brandon_lee@ucsb.edu");
	FILE *fp = popen(command, "w");
	pclose(fp);
	free(command);
	return;
}

int fuse_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	char message[size];
	strcpy(message, buf);
	send_mail(message);
	return sizeof(message);
}

int fuse_flush(const char* c, struct fuse_file_info* f) {
	return 0;
}

int fuse_create(const char* path, mode_t mode, struct fuse_file_info *fi){
	(void) mode;
	(void) fi;
 // Check if the file already exists
    if (find_path(path) != NULL)
        return -EEXIST;  // File already exists

    // Find an empty slot in open_files array
    int i;
    for (i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        if (open_files[i].filename.empty()) {
            break;
        }
    }

    if (i == OPEN_FILE_LIST_SIZE)
        return -ENOSPC;  // No space left

    // Allocate and initialize the new file
    open_files[i].filename = strdup(path);
    if (open_files[i].filename.empty())
        return -ENOMEM;  // Memory allocation failed

	return 0;
}

int fuse_utimens(const char* path,const struct timespec tv[2], struct fuse_file_info *fi) {
	return 0;
}

const struct fuse_operations hello_oper = {
    .getattr    = fuse_getattr,
    .open       = fuse_open,
    // .read       = fuse_read,
    .write      = fuse_write,
    .flush      = fuse_flush,
    .getxattr   = fuse_getxattr,
    .readdir    = fuse_readdir,
    .init       = fuse_init,
    .create     = fuse_create,
    .utimens 	= fuse_utimens,
};
