#define FUSE_USE_VERSION 31
#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include "utils.h"


void testCommand(const char* cmd, bool ignoreError) {
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