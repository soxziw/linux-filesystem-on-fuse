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

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};


class FuseTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};


TEST_F(FuseTest, FuseOpen_newFile) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }
    options.filename = "newfile";

    struct fuse_file_info fi;
    int res = fuse_open("/newfile", &fi);
    ASSERT_EQ(res, 0);
}

TEST_F(FuseTest, FuseOpen_brandNewFile) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }
    options.filename = "newfile";

    struct fuse_file_info fi;
    int res = fuse_open("/brandnewfile", &fi);
    ASSERT_EQ(res, -ENOENT);
}

TEST_F(FuseTest, FuseCreate_ExistFile) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }
    open_files[0].filename = "testfile_fuseopen_existfile";

    struct fuse_file_info fi;
    int res = fuse_create("testfile_fuseopen_existfile", 0, &fi);
    ASSERT_EQ(res, -EEXIST);
}

TEST_F(FuseTest, FuseCreate_CreateFile) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }
    struct fuse_file_info fi;
    int res = fuse_create("testfile_fuseopen_createfile", 0, &fi);
    ASSERT_EQ(res, 0);
}

TEST_F(FuseTest, FuseWrite_Write) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }

    struct fuse_file_info fi;
    std::string message = "Test message";
    int res = fuse_write("testfile_fusewrite_write", message.c_str(), message.size(), 0, &fi);
    
    ASSERT_EQ(res, message.size());
}


