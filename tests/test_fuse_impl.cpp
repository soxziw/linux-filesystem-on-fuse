#define FUSE_USE_VERSION 31
#include <gtest/gtest.h>
#include <string.h>
#include <errno.h>
#include <fuse.h>
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
    const char* mountpoint = "/tmp/mountdir"; // 挂载点
    void SetUp() override {
        // 创建挂载点目录
        mkdir(mountpoint, 0755);

        // 初始化测试变量
        options.filename = strdup("newfile");
        options.contents = strdup("Hello World!\n");
        memset(open_files, 0, sizeof(open_files));

        // 挂载 FUSE 文件系统
        // 这假设你已经有了一个 fuse_main 函数来处理 FUSE 调用
        char* fuse_argv[] = {
            (char*)"270FileSystemTests",
            (char*)mountpoint
        };
        int fuse_argc = sizeof(fuse_argv) / sizeof(fuse_argv[0]) - 1;
        struct fuse_args args = FUSE_ARGS_INIT(fuse_argc, fuse_argv);
        if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
            return;


        for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
            open_files[i].filename = NULL;
        }

        open_files[0].filename = (char*)calloc(strlen("newfile")+1, sizeof(char));
        memcpy(open_files[0].filename, "newfile", strlen("newfile"));
        // 启动 FUSE
        int ret = fuse_main(fuse_argc, fuse_argv, &hello_oper, NULL);
        fuse_opt_free_args(&args);
    }

    void TearDown() override {
        // 卸载 FUSE 文件系统
        // 使用 fusermount 来卸载
        std::string command = "fusermount -u " + std::string(mountpoint);
        system(command.c_str());

        // 删除挂载点目录
        rmdir(mountpoint);

        // 清理
        free((void*)options.filename);
        free((void*)options.contents);
        for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
            free(open_files[i].filename);
        }
    }
};

// 下面是测试用例...

TEST_F(FuseTest, FuseOpen_CreatesWritableFileWhenNotExist) {
    struct fuse_file_info fi;
    fi.flags = O_CREAT | O_RDWR;

    int res = fuse_open("brandnewfile", &fi);
    ASSERT_EQ(res, 0);  // 确保没有错误发生
    ASSERT_STREQ(open_files[1].filename, "brandnewfile");  // 检查文件是否被创建
}

TEST_F(FuseTest, FuseOpen_CreatesReadOnlyFileWhenNotExist) {
    struct fuse_file_info fi;
    fi.flags = O_CREAT | O_RDONLY;

    int res = fuse_open("brandnewfile", &fi);
    ASSERT_EQ(res, 0);  // 确保没有错误发生
    ASSERT_STREQ(open_files[1].filename, "brandnewfile");  // 检查文件是否被创建
}

TEST_F(FuseTest, FuseWrite_ValidWrite) {
    struct fuse_file_info fi;
    fi.flags = O_CREAT | O_RDWR;

    // 首先打开文件
    fuse_open("brandnewfile", &fi);

    // 现在向文件写入
    const char *message = "Test message";
    int res = fuse_write("brandnewfile", message, strlen(message), 0, &fi);
    
    ASSERT_EQ(res, strlen(message));  // 应返回写入的字节数
}

TEST_F(FuseTest, FuseWrite_InvalidFile) {
    struct fuse_file_info fi;
    fi.flags = O_CREAT | O_RDWR;

    // 尝试向未打开的文件写入
    const char *message = "Test message";
    int res = fuse_write("/nonexistentfile", message, strlen(message), 0, &fi);
    
    ASSERT_EQ(res, -EBADF);  // 期望返回坏文件描述符错误
}


