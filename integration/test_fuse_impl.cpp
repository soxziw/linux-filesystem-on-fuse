#define FUSE_USE_VERSION 31
#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include "fuse_impl.h"

class FuseIntegration : public ::testing::Test {
protected:
    const char *mountpoint = "/tmp/mountdir";
    void SetUp() override {
        std::string mkdir_cmd = "mkdir " + (std::string)mountpoint;
        system(mkdir_cmd.c_str());

        std::string mount_cmd = "./270FileSystemProject " + (std::string)mountpoint;
        system(mount_cmd.c_str());
    }

    void TearDown() override {
        std::string unmount_cmd = "fusermount3 -u -z " + (std::string)mountpoint;
        system(unmount_cmd.c_str());

        std::string rmdir_cmd = "rm -r " + (std::string)mountpoint;
        system(rmdir_cmd.c_str());
    }
};


TEST_F(FuseIntegration, WriteExistFile) {
    const char *existFile = "/tmp/mountdir/newfile";
    const char *content = "write exist file\n";

    int fd = open(existFile, O_RDWR);
    ASSERT_GE(fd, 0);

    ssize_t bytes_written = write(fd, content, strlen(content));
    ASSERT_EQ((size_t)bytes_written, strlen(content));
}


TEST_F(FuseIntegration, WriteNewFile) {
    const char *newFile = "/tmp/mountdir/new_file";
    const char *content = "write new file\n";

    int fd = open(newFile, O_WRONLY | O_CREAT, 0666);
    ASSERT_GE(fd, 0);

    ssize_t bytes_written = write(fd, content, strlen(content));
    ASSERT_EQ((size_t)bytes_written, strlen(content));
}


