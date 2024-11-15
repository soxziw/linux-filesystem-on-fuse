// test_layer1_end_to_end.cpp

#include "gtest/gtest.h"
#include <cstring>
#include <ctime>
#include <algorithm>
#include "layer0/disk_interface.hpp"
#include "layer1/data_structs.hpp" // Include your Layer 1 header file


TEST(FilesystemTest, FullFileCycle) {
    in_mem_disk[FILE_SYS_SIZE] = {};
    // Initialize the filesystem with i_list_size = 10 blocks and d_block_size = 10 blocks
    ASSERT_EQ(init_fs(10, 10), 0);

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

TEST(FilesystemTest, IndirectBlocks) {
    in_mem_disk[FILE_SYS_SIZE] = {};
    // Initialize the filesystem
    ASSERT_EQ(init_fs(10, 10), 0);

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


TEST(FilesystemTest, FileDeletionAndFreeList) {
    in_mem_disk[FILE_SYS_SIZE] = {};
    // Initialize the filesystem
    ASSERT_EQ(init_fs(10, 10), 0);

    // Allocate an inode for a regular file (file type = 0)
    int inode_num = alloc_inode(0);
    ASSERT_GE(inode_num, 0);

    // Allocate multiple blocks
    long block_num_1 = alloc_block();
    long block_num_2 = alloc_block();
    ASSERT_GT(block_num_1, 0);
    ASSERT_GT(block_num_2, 0);

    // Write blocks to inode
    ASSERT_EQ(write_inode(inode_num, block_num_1, 0), 0);
    ASSERT_EQ(write_inode(inode_num, block_num_2, 1), 0);

    // Verify the inode data
    Inode inode;
    ASSERT_EQ(read_inode(inode_num, inode), 0);
    ASSERT_EQ(inode.block_addrs[0], block_num_1);
    ASSERT_EQ(inode.block_addrs[1], block_num_2);

    // Delete blocks from the inode
    ASSERT_EQ(delete_from_inode(inode_num, block_num_1), 0);
    ASSERT_EQ(delete_from_inode(inode_num, block_num_2), 0);

    // Free the blocks
    ASSERT_EQ(free_block(block_num_1), 0);
    ASSERT_EQ(free_block(block_num_2), 0);

    // Free the inode
    ASSERT_EQ(free_inode(inode_num), 0);

    // Read the free list and check if blocks are returned
    std::vector<Block> free_list;
    ASSERT_EQ(read_free_list(free_list), 0);
    ASSERT_TRUE(std::find_if(free_list.begin(), free_list.end(),
                             [&](const Block& b) { return std::find(std::begin(b.ind_blocks), std::end(b.ind_blocks), block_num_1) != std::end(b.ind_blocks); }) != free_list.end());
    ASSERT_TRUE(std::find_if(free_list.begin(), free_list.end(),
                             [&](const Block& b) { return std::find(std::begin(b.ind_blocks), std::end(b.ind_blocks), block_num_2) != std::end(b.ind_blocks); }) != free_list.end());

    // Check if the inode is freed
    Inode tmp_inode;
    ASSERT_LT(read_inode(inode_num, tmp_inode), 0);
}

TEST(FilesystemTest, FreeListOperations) {
    in_mem_disk[FILE_SYS_SIZE] = {};
    // Initialize the filesystem
    ASSERT_EQ(init_fs(10, 10), 0);

    // Allocate a block and free it
    long block_num = alloc_block();
    ASSERT_GT(block_num, 0);
    ASSERT_EQ(free_block(block_num), 0);

    // Verify that the block appears in the free list
    std::vector<Block> free_list;
    ASSERT_EQ(read_free_list(free_list), 0);
    ASSERT_TRUE(std::find_if(free_list.begin(), free_list.end(),
                             [&](const Block& b) { return std::find(std::begin(b.ind_blocks), std::end(b.ind_blocks), block_num) != std::end(b.ind_blocks); }) != free_list.end());
}
