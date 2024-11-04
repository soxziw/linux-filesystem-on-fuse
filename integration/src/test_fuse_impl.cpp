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
#include "utils.h"


/**
 * Integration tests.
 * Googletest will call SetUp() before each test case, and TearDown() after each test case.
 */
class FuseIntegration : public ::testing::Test {
protected:
    const char* mountpoint = "/tmp/mountdir";

    /**
     * Create a new directory as mountpoint and run ./fuse to mount.
     * @return
     */
    void SetUp() override {
        std::string mkdir_cmd = "mkdir " + (std::string)mountpoint;
        testCommand(mkdir_cmd.c_str());

        std::string mount_cmd = "./fuse " + (std::string)mountpoint;
        testCommand(mount_cmd.c_str());
    }

    /**
     * Unmount the mountpoint and remove the directory.
     * @return
     */
    void TearDown() override {
        std::string unmount_cmd = "fusermount3 -u -z " + (std::string)mountpoint;
        testCommand(unmount_cmd.c_str());

        std::string rmdir_cmd = "rm -r " + (std::string)mountpoint;
        testCommand(rmdir_cmd.c_str());
    }
};


// TEST_F(FuseIntegration, WriteExistFile) {
//     const char* existFile = "/tmp/mountdir/newfile";
//     const char* content = "write exist file\n";

//     int fd = open(existFile, O_RDWR);
//     ASSERT_GE(fd, 0);

//     ssize_t bytes_written = write(fd, content, strlen(content));
//     ASSERT_EQ((size_t)bytes_written, strlen(content));
// }


/**
 * Test 'open' a new file and 'write' to it.
 */
TEST_F(FuseIntegration, WriteNewFile) {
    const char* newFile = "/tmp/mountdir/new_file";
    const char* content = "write new file\n";

    int fd = open(newFile, O_WRONLY | O_CREAT, 0666);
    ASSERT_GE(fd, 0);

    ssize_t bytes_written = write(fd, content, strlen(content));
    ASSERT_EQ((size_t)bytes_written, strlen(content));
}


