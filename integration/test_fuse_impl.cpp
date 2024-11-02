#define FUSE_USE_VERSION 31
#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <pthread.h>
#include "fuse_impl.h"

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

class FuseIntegration : public ::testing::Test {
protected:
    const char *mountpoint = "/tmp/mountdir";

    /**
     * Create a new process to call fuse_main in order to load mount point.
     */
    void start_fuse() {
        options.filename = "newfile";
        options.contents = "Hello World!\n";
        options.show_help = false;

        // for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        //     open_files[i].filename.clear();
        // }
        // open_files[0].filename = "newfile";

        int fuse_argc = 2;
        char* fuse_argv[] = {
            (char *)"./270FileSystemIntegrations",
            (char *)mountpoint,
            NULL
        };

        struct fuse_args args = FUSE_ARGS_INIT(fuse_argc, fuse_argv);
        if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
            perror("Failed to parse FUSE options.");
            return;
        }
        
        int ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
        if (ret != 0) {
            perror("fuse_main failed with error code: " + ret);
        }
        fuse_opt_free_args(&args);
        _exit(0);
    }

    void SetUp() override {
        std::string mkdir_command = "mkdir " + (std::string)mountpoint;
        system(mkdir_command.c_str());
        
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            return;
        } else if (pid == 0) {
            start_fuse();
        } else {
            wait(NULL);
        }
    }

    void TearDown() override {
        std::string unmount_command = "fusermount3 -u -z " + (std::string)mountpoint;
        system(unmount_command.c_str());

        std::string rmdir_command = "rm -r " + (std::string)mountpoint;
        system(rmdir_command.c_str());

        options.filename.clear();
        options.contents.clear();
        for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
            open_files[i].filename.clear();
        }
    }
};
void wef(){
}
TEST_F(FuseIntegration, WriteExistFile) {
    wef();
    const char *existFile = "/tmp/mountdir/exist_file";
    const char *content = "write exist file\n";
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }
    open_files[6].filename = existFile;

    int fd = open(existFile, O_RDWR);
    ASSERT_EQ(fd, 6);

    ssize_t bytes_written = write(fd, content, strlen(content));
    ASSERT_EQ((size_t)bytes_written, strlen(content));
}
void wnf(){
}
TEST_F(FuseIntegration, WriteNewFile) {
    wnf();
    const char *newFile = "/tmp/mountdir/new_file";
    const char *content = "write new file\n";
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }

    int fd = open(newFile, O_WRONLY | O_CREAT, 0666);
    ASSERT_GE(fd, 0);

    ssize_t bytes_written = write(fd, content, strlen(content));
    ASSERT_EQ((size_t)bytes_written, strlen(content));
}


