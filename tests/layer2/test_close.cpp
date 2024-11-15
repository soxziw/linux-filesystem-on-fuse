#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include "layer2/close.h"

TEST(FuseTest, Layer2_Close_InvalidInode) {
    int status = close(nullptr);
    ASSERT_EQ(status, -ENOENT);
}

TEST(FuseTest, Layer2_Close_ValidInode) {
    Inode* file_inode = new Inode;
    int status = close(file_inode);
    ASSERT_EQ(status, 0);
}