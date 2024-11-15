#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include "layer2/open.h"

TEST(FuseTest, Layer2_Open_InvalidInode) {
    int status = open(nullptr, 0, nullptr);
    ASSERT_EQ(status, -ENOENT);
}

TEST(FuseTest, Layer2_Open_ValidInodeInvalidPermission) {
    Inode* file_inode = new Inode;
    int status = open(file_inode, (1 << 10) - 1, new fuse_context);
    ASSERT_EQ(status, -EACCES);
}

TEST(FuseTest, Layer2_Open_ValidInode) {
    Inode* file_inode = new Inode;
    int status = open(file_inode, 0, new fuse_context);
    ASSERT_EQ(status, 0);
}