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

void testCommand(const char* cmd, bool ignoreError = false) {
    FILE* fp = popen(cmd, "w");
    ASSERT_NE(fp, nullptr) << "[" << cmd << "] Failed to run command";

    int status = pclose(fp);
    ASSERT_NE(status, -1) << "[" << cmd << "] pclose() failed";

    if (!ignoreError) {
        if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            EXPECT_EQ(exitStatus, 0) << "[" << cmd << "] Command exited with error status: " << exitStatus;
        } else {
            FAIL() << "[" << cmd << "] Command did not terminate normally.";
        }
    }
}


class FuseIntegration : public ::testing::Test {
protected:
    const char* mountpoint = "/tmp/mountdir";
    void SetUp() override {
        std::string mkdir_cmd = "mkdir " + (std::string)mountpoint;
        testCommand(mkdir_cmd.c_str());

        std::string mount_cmd = "./270FileSystemProject " + (std::string)mountpoint;
        testCommand(mount_cmd.c_str());
    }

    void TearDown() override {
        std::string unmount_cmd = "fusermount3 -u -z " + (std::string)mountpoint;
        testCommand(unmount_cmd.c_str());

        std::string rmdir_cmd = "rm -r " + (std::string)mountpoint;
        testCommand(rmdir_cmd.c_str());
    }
};


TEST_F(FuseIntegration, TouchToCreate) {
    const char* touchFile = "/tmp/mountdir/touch_file";

    std::string touch_cmd = "touch " + (std::string)touchFile;
    testCommand(touch_cmd.c_str());

    std::string test_cmd = "test " + (std::string)touchFile;
    testCommand(test_cmd.c_str());
}


TEST_F(FuseIntegration, EchoToCreate) {
    const char* echoFile = "/tmp/mountdir/echo_file";
    const char* content = "echo comment\n";

    std::string echo_cmd = "echo " + (std::string)content + " > " + (std::string)echoFile;
    testCommand(echo_cmd.c_str());

    std::string test_cmd = "test " + (std::string)echoFile;
    testCommand(test_cmd.c_str());
}


