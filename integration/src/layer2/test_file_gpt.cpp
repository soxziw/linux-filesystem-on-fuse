#include <gtest/gtest.h>
#include "layer0/disk_interface.hpp"
#include "layer1/data_structs.hpp" // Include your filesystem header here
#include "layer2/translate_pos.h"
#include "layer2/open.h"
#include "layer2/close.h"
#include "layer2/read.h"
#include "layer2/write.h"

class FileSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize the file system with 10 inode blocks and 100 data blocks
        init_fs(10, 100);
    }

    void TearDown() override {
        // Clear the in-memory disk after each test
        memset(in_mem_disk, 0, sizeof(in_mem_disk));
    }
};

// Test: translate_pos - Direct Block
TEST_F(FileSystemTest, TranslatePosDirectBlock) {
    int inode_num = alloc_inode(FILE_TYPE::REG_FILE);
    ASSERT_GT(inode_num, 0);

    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);

    Position pos;
    int status = translate_pos(&inode, &pos, 0, true); // Write mode
    ASSERT_EQ(status, 0);
    ASSERT_GT(pos.block_num, 0);
    ASSERT_EQ(pos.offset, 0);

    free_inode(inode_num);
}

// Test: read - Read Data from File
TEST_F(FileSystemTest, ReadFile) {
    int inode_num = alloc_inode(FILE_TYPE::REG_FILE);
    ASSERT_GT(inode_num, 0);

    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);

    const char* write_data = "Hello, World!";
    size_t write_size = strlen(write_data);
    ASSERT_EQ(write(&inode, write_data, write_size, 0), 0);

    char read_buf[BLOCK_SIZE] = {0};
    ASSERT_EQ(read(&inode, read_buf, write_size, 0), 0);
    ASSERT_STREQ(read_buf, write_data);

    free_inode(inode_num);
}

// Test: write - Write Data to File
TEST_F(FileSystemTest, WriteFile) {
    int inode_num = alloc_inode(FILE_TYPE::REG_FILE);
    ASSERT_GT(inode_num, 0);

    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);

    const char* write_data = "Filesystem E2E Test!";
    size_t write_size = strlen(write_data);
    ASSERT_EQ(write(&inode, write_data, write_size, 0), 0);

    char read_buf[BLOCK_SIZE] = {0};
    ASSERT_EQ(read(&inode, read_buf, write_size, 0), 0);
    ASSERT_STREQ(read_buf, write_data);

    free_inode(inode_num);
}

// Test: open - Check Permissions
TEST_F(FileSystemTest, OpenFilePermissions) {
    int inode_num = alloc_inode(FILE_TYPE::REG_FILE);
    ASSERT_GT(inode_num, 0);

    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);

    // Set metadata for permissions
    inode.m_data.owner_id = 1000;
    inode.m_data.group_id = 100;
    inode.m_data.mode = 0777;
    ASSERT_EQ(write_inode_mdata(inode.m_data, inode_num), 0);

    fuse_context context = { .uid = 1000, .gid = 100, .pid = 0 }; // Simulate a user
    ASSERT_EQ(open(&inode, 07, &context), 0);

    context.uid = 2000;
    ASSERT_EQ(open(&inode, 07, &context), 0); // Other permission should work

    free_inode(inode_num);
}

// Test: close - Close File
TEST_F(FileSystemTest, CloseFile) {
    int inode_num = alloc_inode(FILE_TYPE::REG_FILE);
    ASSERT_GT(inode_num, 0);

    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);

    ASSERT_EQ(close(&inode), 0); // Close always succeeds
    free_inode(inode_num);
}

// Test: read + write integration
TEST_F(FileSystemTest, ReadWriteIntegration) {
    int inode_num = alloc_inode(FILE_TYPE::REG_FILE);
    ASSERT_GT(inode_num, 0);

    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);

    const char* write_data = "Integrated Test for Read and Write";
    size_t write_size = strlen(write_data);
    ASSERT_EQ(write(&inode, write_data, write_size, 0), 0);

    char read_buf[BLOCK_SIZE] = {0};
    ASSERT_EQ(read(&inode, read_buf, write_size, 0), 0);
    ASSERT_STREQ(read_buf, write_data);

    free_inode(inode_num);
}

// Test: open + write + read + close integration
TEST_F(FileSystemTest, OpenWriteReadCloseIntegration) {
    int inode_num = alloc_inode(FILE_TYPE::REG_FILE);
    ASSERT_GT(inode_num, 0);

    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);

    fuse_context context = { .uid = 0, .gid = 0, .pid = 0 }; // Root user
    ASSERT_EQ(open(&inode, 07, &context), 0);

    const char* write_data = "Filesystem Full Integration Test!";
    size_t write_size = strlen(write_data);
    ASSERT_EQ(write(&inode, write_data, write_size, 0), 0);

    char read_buf[BLOCK_SIZE] = {0};
    ASSERT_EQ(read(&inode, read_buf, write_size, 0), 0);
    ASSERT_STREQ(read_buf, write_data);

    ASSERT_EQ(close(&inode), 0);
    free_inode(inode_num);
}
