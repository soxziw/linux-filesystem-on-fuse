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

// #define OPTION(t, p)                           \
//     { t, offsetof(struct options, p), 1 }
// static const struct fuse_opt option_spec[] = {
// 	OPTION("--name=%s", filename),
// 	OPTION("--contents=%s", contents),
// 	OPTION("-h", show_help),
// 	OPTION("--help", show_help),
// 	FUSE_OPT_END
// };

/**
 * Test 'fuse_open' a exist file.
 */
TEST(FuseTest, FuseOpen_ExistFile) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }
    open_files[0].filename = "exist_file";

    struct fuse_file_info fi;
    int res = fuse_open("exist_file", &fi);
    ASSERT_EQ(res, 0);
}


/**
 * Test 'fuse_open' a new file.
 */
TEST(FuseTest, FuseOpen_NewFile) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }
    open_files[0].filename = "exist_file";

    struct fuse_file_info fi;
    int res = fuse_open("new_file", &fi);
    ASSERT_EQ(res, -ENOENT);
}


/**
 * Test 'fuse_create' a exist file.
 */
TEST(FuseTest, FuseCreate_ExistFile) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }
    open_files[0].filename = "exist_file";

    struct fuse_file_info fi;
    int res = fuse_create("exist_file", 0, &fi);
    ASSERT_EQ(res, -EEXIST);
}


/**
 * Test 'fuse_create' a new file.
 */
TEST(FuseTest, FuseCreate_CreateFile) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }
    struct fuse_file_info fi;
    int res = fuse_create("new_file", 0, &fi);
    ASSERT_EQ(res, 0);
}

/**
 * Test 'fuse_write' a file.
 */
TEST(FuseTest, FuseWrite_Write) {
    for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
        open_files[i].filename.clear();
    }

    struct fuse_file_info fi;
    std::string message = "FuseWrite_Write Test message";
    int res = fuse_write("write_file", message.c_str(), message.size(), 0, &fi);
    ASSERT_EQ(res, message.size());
}


