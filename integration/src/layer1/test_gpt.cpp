#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include "layer0/disk_interface.hpp"
#include "layer1/data_structs.hpp"

// Helper to initialize the test environment
class FileSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize the file system with 100 inode blocks and 1000 data blocks
        init_fs(10, 100);
    }

    void TearDown() override {
        // Clear in-memory disk after each test
        memset(in_mem_disk, 0, sizeof(in_mem_disk));
    }
};

// Test: SuperBlock Initialization
TEST_F(FileSystemTest, SuperBlockInitialization) {
    Block superblock;
    int res = read_from_block((char*)&superblock, 0, 0, BLOCK_SIZE);
    ASSERT_EQ(res, BLOCK_SIZE);

    ASSERT_EQ(superblock.super.i_list_size, 10);
    ASSERT_EQ(superblock.super.d_block_size, 100);
    ASSERT_EQ(superblock.super.root_inode_num, ROOT_INODE_NUM);
    ASSERT_NE(superblock.super.creation_ts, 0);
}

// Test: Inode Allocation and Read
TEST_F(FileSystemTest, InodeAllocationAndRead) {
    int inode_num = alloc_inode(1); // Allocate a regular file inode
    ASSERT_GT(inode_num, 0);

    Inode inode;
    int res = read_inode(inode_num, inode);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(inode.m_data.inode_num, (unsigned long)inode_num);
    ASSERT_TRUE(inode.m_data.is_allocated);
    ASSERT_EQ(inode.m_data.file_type, 1);
}

// Test: Inode Deallocation
TEST_F(FileSystemTest, InodeDeallocation) {
    int inode_num = alloc_inode(1);
    ASSERT_GT(inode_num, 0);

    int res = free_inode(inode_num);
    ASSERT_EQ(res, 0);

    Inode inode;
    res = read_inode(inode_num, inode);
    ASSERT_EQ(res, -4); // Inode should now be marked free
}

// Test: Block Allocation and Freeing
TEST_F(FileSystemTest, BlockAllocationAndFreeing) {
    long block_num = alloc_block();
    ASSERT_GT(block_num, 0);

    int res = free_block(block_num);
    ASSERT_EQ(res, 0);
}

// Test: Write and Read Data from Inode
TEST_F(FileSystemTest, WriteAndReadInode) {
    int inode_num = alloc_inode(1);
    ASSERT_GT(inode_num, 0);

    long block_num = alloc_block();
    ASSERT_GT(block_num, 0);

    int res = write_inode(inode_num, block_num, 0); // Write block_num to direct block[0]
    ASSERT_EQ(res, 0);

    Inode inode;
    res = read_inode(inode_num, inode);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(inode.block_addrs[0], block_num);
}

// Test: Freeing Indirect Blocks
TEST_F(FileSystemTest, FreeIndirectBlocks) {
    long ind_block = alloc_block();
    ASSERT_GT(ind_block, 0);

    // Simulate indirect block allocation
    write_to_block((const char*)&ind_block, ind_block, 0, sizeof(long));

    int res = free_ind_block(ind_block);
    ASSERT_EQ(res, 0);
}

// Test 1: Initialize Filesystem
TEST_F(FileSystemTest, InitFileSystem) {
    Block sup_block;
    read_from_block((char*)&sup_block, 0, 0, BLOCK_SIZE);

    ASSERT_EQ(sup_block.super.i_list_size, 10);
    ASSERT_EQ(sup_block.super.d_block_size, 100);
    ASSERT_EQ(sup_block.super.root_inode_num, ROOT_INODE_NUM);
}

// Test 2: Allocate and Free Inode
TEST_F(FileSystemTest, AllocFreeInode) {
    int inode_num = alloc_inode(0);
    ASSERT_GT(inode_num, 0);

    Inode inode;
    int status = read_inode(inode_num, inode);
    ASSERT_EQ(status, 0);
    ASSERT_TRUE(inode.m_data.is_allocated);
    ASSERT_EQ(inode.m_data.file_type, 0);

    status = free_inode(inode_num);
    ASSERT_EQ(status, 0);

    status = read_inode(inode_num, inode);
    ASSERT_EQ(status, -4);  // Check that the inode is deallocated
}

// Test 3: Allocate and Free Data Block
TEST_F(FileSystemTest, AllocFreeBlock) {
    long block_num = alloc_block();
    ASSERT_GT(block_num, 0);

    int status = free_block(block_num);
    ASSERT_EQ(status, 0);
}

// Test 4: Write and Read Data in Block
TEST_F(FileSystemTest, WriteReadBlock) {
    char write_buf[BLOCK_SIZE] = "Hello, Filesystem!";
    char read_buf[BLOCK_SIZE] = {0};

    long block_num = alloc_block();
    ASSERT_GT(block_num, 0);

    int status = write_to_block(write_buf, block_num, 0, strlen(write_buf));
    ASSERT_EQ(status, strlen(write_buf));

    status = read_from_block(read_buf, block_num, 0, BLOCK_SIZE);
    ASSERT_EQ(status, BLOCK_SIZE);
    ASSERT_STREQ(read_buf, write_buf);

    free_block(block_num);
}

// Test 5: Root Inode Initialization
TEST_F(FileSystemTest, RootInodeInitialization) {
    Inode root_inode;
    int status = read_inode(ROOT_INODE_NUM, root_inode);
    ASSERT_EQ(status, 0);

    ASSERT_TRUE(root_inode.m_data.is_allocated);
    ASSERT_EQ(root_inode.m_data.file_type, 1);
    ASSERT_EQ(root_inode.m_data.inode_num, ROOT_INODE_NUM);
}

// Test 6: Recursive Block Deallocation (Indirect Blocks)
TEST_F(FileSystemTest, RecursiveBlockDeallocation) {
    long indirect_block = alloc_block();
    long data_block = alloc_block();

    ASSERT_GT(indirect_block, 0);
    ASSERT_GT(data_block, 0);

    Block block;
    block.ind_blocks[0] = data_block;
    write_to_block((const char*)&block, indirect_block, 0, BLOCK_SIZE);

    int status = free_ind_block(indirect_block);
    ASSERT_EQ(status, 0);
}

// Test 7: Metadata Update
TEST_F(FileSystemTest, WriteInodeMetadata) {
    int inode_num = alloc_inode(0);
    ASSERT_GT(inode_num, 0);

    InodeMData new_metadata = {
        .dev_num = 1,
        .inode_num = (long unsigned int)inode_num,
        .mode = 0777,
        .num_hard_links = 1,
        .owner_id = 1000,
        .group_id = 1000,
        .file_size = 1024,
        .file_type = 0,
        .is_allocated = true,
        .atime = time(nullptr),
        .creation_time = time(nullptr),
        .mtime = time(nullptr),
        .ctime = time(nullptr),
    };

    int status = write_inode_mdata(new_metadata, inode_num);
    ASSERT_EQ(status, 0);

    Inode inode;
    status = read_inode(inode_num, inode);
    ASSERT_EQ(status, 0);
    ASSERT_TRUE(inode.m_data == new_metadata);

    free_inode(inode_num);
}

TEST_F(FileSystemTest, FullFileCycle) {
    // Allocate an inode for a regular file (file type = 0)
    int inode_num = alloc_inode(0);
    ASSERT_GE(inode_num, 0);  // Ensure allocation was successful

    // Read the inode to verify it was properly created
    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);

    // Allocate a block for this inode
    long block_num = alloc_block();
    ASSERT_GT(block_num, 0);  // Ensure block allocation was successful

    // Write this block to the inode (block index 0)
    ASSERT_EQ(write_inode(inode_num, block_num, 0), 0);

    // Verify the block is properly written by checking the inode
    ASSERT_EQ(read_inode(inode_num, inode), 0);
    ASSERT_EQ(inode.block_addrs[0], block_num);

    // Free the block
    ASSERT_EQ(free_block(block_num), 0);

    // Free the inode  
    ASSERT_EQ(free_inode(inode_num), 0);

    // Check that the inode is properly freed by attempting to read it    
    ASSERT_LT(read_inode(inode_num, inode), 0);
}

TEST_F(FileSystemTest, IndirectBlocks) {
    // Allocate an inode for a regular file (file type = 0)
    int inode_num = alloc_inode(0);
    ASSERT_GE(inode_num, 0);

    // Read the inode
    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);

    // Allocate a block (direct block)
    long block_num_1 = alloc_block();
    ASSERT_GT(block_num_1, 0);

    // Allocate an indirect block
    long ind_block = alloc_block();
    ASSERT_GT(ind_block, 0);

    // Allocate additional blocks to be linked via indirect block
    long block_num_2 = alloc_block();
    long block_num_3 = alloc_block();

    ASSERT_GT(block_num_3, block_num_2);
    ASSERT_GT(block_num_2, 0);
    ASSERT_GT(block_num_3, 0);

    // Set the indirect block to point to the new blocks
    inode.block_addrs[12] = ind_block;
    // Here, assuming the indirect block just points to these new blocks
    Block ind_blk;
    ind_blk.ind_blocks[0] = block_num_2;
    ind_blk.ind_blocks[1] = block_num_3;

    // Write the indirect block to the inode
    ASSERT_EQ(write_inode(inode_num, ind_block, 12), 0);

    // Verify the inode's indirect block
    ASSERT_EQ(read_inode(inode_num, inode), 0);
    ASSERT_EQ(inode.block_addrs[12], ind_block);

    // Clean up
    ASSERT_EQ(free_block(block_num_1), 0);
    ASSERT_EQ(free_block(block_num_2), 0);
    ASSERT_EQ(free_block(block_num_3), 0);
    ASSERT_EQ(free_block(ind_block), 0);
    ASSERT_EQ(free_inode(inode_num), 0);
}