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


#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

typedef struct open_file_info {
	char* filename;
	int flags;
} open_file_info;

#define OPEN_FILE_LIST_SIZE 100
open_file_info open_files[OPEN_FILE_LIST_SIZE];

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void* fuse_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

static int fuse_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
	} else if (strcmp(path+1, options.filename) == 0) {
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(options.contents);
	} else
		res = -ENOENT;

	return res;
}

static int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags)
{
    (void) offset;
    (void) fi;

    // Ensure the path is the root directory
    if (strcmp(path, "/") != 0)
        return -ENOENT;
;
    // Prepare stat structure for directory entries
    struct stat st;
    memset(&st, 0, sizeof(st));

    // Add "." and ".." entries
    st.st_mode = S_IFDIR | 0755;
    filler(buf, ".", &st, 0, 0);
    filler(buf, "..", &st, 0, 0);

    // Add the "hello" file entry
    st.st_mode = S_IFREG | 0777;
    st.st_nlink = 1;
    st.st_size = strlen(options.contents);
    filler(buf, options.filename, &st, 0, 0);

    return 0;
}
static int fuse_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
	int rd_only = 0, wr_only = 0, rdwr = 0, creat = 0; 
	if ((fi->flags & O_RDONLY) == O_RDONLY) rd_only = 1;
	if ((fi->flags & O_WRONLY) == O_WRONLY) wr_only = 1;
	if ((fi->flags & O_RDWR) == O_RDWR) rdwr = 1;
	if ((fi->flags & O_CREAT) == O_CREAT) creat = 1;

	if (O_CREAT) {
		//first look for file, and if not create a file
		int i = 0;
		while (i < OPEN_FILE_LIST_SIZE && open_files[i].filename!=NULL){
			if (strcmp(open_files[i].filename, path) == 0) { //file is already opened
				return -EBUSY;
			}
			i += 1;
		}
		open_files[i].filename = (char*)calloc(strlen(path)+1, sizeof(char));
		memcpy(open_files[i].filename, path, strlen(path));
	}
	return 0;
}

static int fuse_open(const char *path, struct fuse_file_info *fi) {
	// if (strcmp(path+1, options.filename) != 0)
	// 	return -ENOENT;
	int rd_only = 0, wr_only = 0, rdwr = 0, creat = 0; 
	if ((fi->flags & O_RDONLY) == O_RDONLY) rd_only = 1;
	if ((fi->flags & O_WRONLY) == O_WRONLY) wr_only = 1;
	if ((fi->flags & O_RDWR) == O_RDWR) rdwr = 1;
	if ((fi->flags & O_CREAT) == O_CREAT) creat = 1;

	if (O_CREAT) {
		//first look for file, and if not create a file
		int i = 0;
		while (i < OPEN_FILE_LIST_SIZE && open_files[i].filename!=NULL){
			if (strcmp(open_files[i].filename, path) == 0) { //file is already opened
				return -EBUSY;
			}
			i += 1;
		}
		open_files[i].filename = (char*)calloc(strlen(path)+1, sizeof(char));
		memcpy(open_files[i].filename, path, strlen(path));
	}

	return 0;
}

static int fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	size_t len;
	(void) fi;
	if(strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	len = strlen(options.contents);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, options.contents + offset, size);
	} else
		size = 0;

	return size;
}

static int fuse_getxattr(const char* path, const char* name, char* value, size_t size) {
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

static int fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	int i = 0;
	// check if file to be written to is opened
	short is_open = 0;
	while (i < OPEN_FILE_LIST_SIZE) {
		if (open_files[i].filename) {
			if (strcmp(open_files[i].filename, path) == 0) {
			is_open = 1;
			break;
			}
		}
		i++;
	}
	if (!is_open) return -EBADF;

	char message[size];
	strcpy(message, buf);
	send_mail(message);
	return sizeof(message);
}

static int fuse_flush(const char* c, struct fuse_file_info* f) {
	return 0;
}

static const struct fuse_operations hello_oper = {
	.init           = fuse_init,
	.getattr	= fuse_getattr,
	.readdir	= fuse_readdir,
	.open		= fuse_open,
	.read		= fuse_read,
	.write 		= fuse_write,
	.getxattr	= fuse_getxattr,
	.flush		= fuse_flush,
	.create		= fuse_create
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("newfile");
	options.contents = strdup("Hello World!\n");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}

	for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
		open_files[i].filename = NULL;
	}

	ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}