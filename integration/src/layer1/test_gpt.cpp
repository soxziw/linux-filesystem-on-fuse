// test_layer1_end_to_end.cpp

#include "gtest/gtest.h"
#include <cstring>
#include <ctime>
#include "layer2/mock_layer1.h" // Include your Layer 1 header file

// Assuming that Layer 0 functions are properly implemented and available.

class Layer1EndToEndTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize the filesystem
        int i_list_size = 1024; // Number of inode blocks
        int d_block_size = 8192; // Number of data blocks
        int res = init_fs(i_list_size, d_block_size);
        ASSERT_EQ(res, 0); // Ensure filesystem initialization succeeds
    }

    void TearDown() override {
        // Cleanup code if necessary
    }
};

// Test case: Create a file, write data, and read it back
TEST_F(Layer1EndToEndTest, CreateFileWriteReadTest) {
    // Step 1: Allocate an inode for a regular file
    short file_type = 0; // Regular file
    int inode_num = alloc_inode(file_type);
    ASSERT_GE(inode_num, 0); // Ensure inode allocation succeeds

    // Step 2: Allocate a data block
    long block_num = alloc_block();
    ASSERT_GE(block_num, 0); // Ensure block allocation succeeds

    // Step 3: Write data to the block
    Block data_block;
    const char* content = "Hello, Layer 1!";
    memset(&data_block, 0, sizeof(Block));
    memcpy(data_block.dir_block, content, strlen(content) + 1);

    int res = write_block(data_block, block_num);
    ASSERT_EQ(res, 0); // Ensure block write succeeds

    // Step 4: Update the inode to point to the data block
    int inode_idx = 0; // First direct block
    res = write_inode(inode_num, block_num, inode_idx);
    ASSERT_EQ(res, 0); // Ensure inode write succeeds

    // Step 5: Update inode metadata
    InodeMData m_data;
    memset(&m_data, 0, sizeof(InodeMData));
    m_data.inode_num = inode_num;
    m_data.file_type = file_type;
    m_data.file_size = strlen(content) + 1;
    m_data.num_allocated_blocks = 1;
    m_data.owner_id = 1000;
    m_data.group_id = 1000;
    m_data.mode = 0644;
    m_data.atime = m_data.mtime = m_data.ctime = m_data.creation_time = std::time(nullptr);

    res = write_inode_mdata(m_data, inode_num);
    ASSERT_EQ(res, 0); // Ensure inode metadata write succeeds

    // Step 6: Read the inode back and verify
    Inode inode;
    res = read_inode(inode_num, inode);
    ASSERT_EQ(res, 0); // Ensure inode read succeeds
    EXPECT_EQ(inode.m_data.file_size, m_data.file_size);
    EXPECT_EQ(inode.block_addrs[inode_idx], block_num);

    // Step 7: Read the data block and verify content
    Block read_block;
    res = read_block(read_block, block_num);
    ASSERT_EQ(res, 0); // Ensure block read succeeds
    EXPECT_STREQ(read_block.dir_block, content);
}

// Test case: Create a directory and verify its entries
TEST_F(Layer1EndToEndTest, CreateDirectoryTest) {
    // Step 1: Allocate an inode for a directory
    short file_type = 1; // Directory
    int dir_inode_num = alloc_inode(file_type);
    ASSERT_GE(dir_inode_num, 0); // Ensure inode allocation succeeds

    // Step 2: Allocate a data block for the directory
    long dir_block_num = alloc_block();
    ASSERT_GE(dir_block_num, 0); // Ensure block allocation succeeds

    // Step 3: Create "." and ".." entries
    Block dir_block;
    memset(&dir_block, 0, sizeof(Block));

    DirEntry dot_entry;
    dot_entry.inode_num = dir_inode_num;
    strcpy(dot_entry.name, ".");
    memcpy(dir_block.dir_block, &dot_entry, sizeof(DirEntry));

    DirEntry dotdot_entry;
    dotdot_entry.inode_num = dir_inode_num; // For simplicity, parent is self
    strcpy(dotdot_entry.name, "..");
    memcpy(dir_block.dir_block + sizeof(DirEntry), &dotdot_entry, sizeof(DirEntry));

    // Step 4: Write the directory block
    int res = write_block(dir_block, dir_block_num);
    ASSERT_EQ(res, 0); // Ensure block write succeeds

    // Step 5: Update the inode to point to the directory block
    int inode_idx = 0; // First direct block
    res = write_inode(dir_inode_num, dir_block_num, inode_idx);
    ASSERT_EQ(res, 0); // Ensure inode write succeeds

    // Step 6: Update inode metadata
    InodeMData m_data;
    memset(&m_data, 0, sizeof(InodeMData));
    m_data.inode_num = dir_inode_num;
    m_data.file_type = file_type;
    m_data.file_size = 2 * sizeof(DirEntry);
    m_data.num_allocated_blocks = 1;
    m_data.owner_id = 1000;
    m_data.group_id = 1000;
    m_data.mode = 0755;
    m_data.atime = m_data.mtime = m_data.ctime = m_data.creation_time = std::time(nullptr);

    res = write_inode_mdata(m_data, dir_inode_num);
    ASSERT_EQ(res, 0); // Ensure inode metadata write succeeds

    // Step 7: Read the inode back and verify
    Inode dir_inode;
    res = read_inode(dir_inode_num, dir_inode);
    ASSERT_EQ(res, 0); // Ensure inode read succeeds
    EXPECT_EQ(dir_inode.m_data.file_size, m_data.file_size);
    EXPECT_EQ(dir_inode.block_addrs[inode_idx], dir_block_num);

    // Step 8: Read the directory block and verify entries
    Block read_dir_block;
    res = read_block(read_dir_block, dir_block_num);
    ASSERT_EQ(res, 0); // Ensure block read succeeds

    DirEntry read_dot_entry;
    memcpy(&read_dot_entry, read_dir_block.dir_block, sizeof(DirEntry));
    EXPECT_EQ(read_dot_entry.inode_num, dir_inode_num);
    EXPECT_STREQ(read_dot_entry.name, ".");

    DirEntry read_dotdot_entry;
    memcpy(&read_dotdot_entry, read_dir_block.dir_block + sizeof(DirEntry), sizeof(DirEntry));
    EXPECT_EQ(read_dotdot_entry.inode_num, dir_inode_num);
    EXPECT_STREQ(read_dotdot_entry.name, "..");
}

// Test case: Write multiple blocks to a file and read them back
TEST_F(Layer1EndToEndTest, MultiBlockFileTest) {
    // Step 1: Allocate an inode for a regular file
    short file_type = 0; // Regular file
    int inode_num = alloc_inode(file_type);
    ASSERT_GE(inode_num, 0); // Ensure inode allocation succeeds

    // Step 2: Allocate multiple data blocks and write data
    const int num_blocks = 5;
    const char* data_contents[num_blocks] = {
        "Data block 1",
        "Data block 2",
        "Data block 3",
        "Data block 4",
        "Data block 5"
    };
    for (int i = 0; i < num_blocks; ++i) {
        long block_num = alloc_block();
        ASSERT_GE(block_num, 0); // Ensure block allocation succeeds

        // Write data to block
        Block data_block;
        memset(&data_block, 0, sizeof(Block));
        memcpy(data_block.dir_block, data_contents[i], strlen(data_contents[i]) + 1);

        int res = write_block(data_block, block_num);
        ASSERT_EQ(res, 0); // Ensure block write succeeds

        // Update inode with block number
        res = write_inode(inode_num, block_num, i);
        ASSERT_EQ(res, 0); // Ensure inode write succeeds
    }

    // Step 3: Update inode metadata
    InodeMData m_data;
    memset(&m_data, 0, sizeof(InodeMData));
    m_data.inode_num = inode_num;
    m_data.file_type = file_type;
    m_data.file_size = num_blocks * BLOCK_SIZE;
    m_data.num_allocated_blocks = num_blocks;
    m_data.owner_id = 1000;
    m_data.group_id = 1000;
    m_data.mode = 0644;
    m_data.atime = m_data.mtime = m_data.ctime = m_data.creation_time = std::time(nullptr);

    int res = write_inode_mdata(m_data, inode_num);
    ASSERT_EQ(res, 0); // Ensure inode metadata write succeeds

    // Step 4: Read inode back and verify
    Inode inode;
    res = read_inode(inode_num, inode);
    ASSERT_EQ(res, 0); // Ensure inode read succeeds
    EXPECT_EQ(inode.m_data.num_allocated_blocks, num_blocks);

    // Step 5: Read data blocks back and verify content
    for (int i = 0; i < num_blocks; ++i) {
        long block_num = inode.block_addrs[i];
        Block read_block;
        res = read_block(read_block, block_num);
        ASSERT_EQ(res, 0); // Ensure block read succeeds
        EXPECT_STREQ(read_block.dir_block, data_contents[i]);
    }
}