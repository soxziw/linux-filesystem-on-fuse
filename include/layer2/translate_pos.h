#include <iostream>
#include <cerrno>
#include "mock_layer1.h"

#define OWNER_MODE 7
#define GROUP_MODE 7 << 3
#define OTHER_MODE 7 << 6


#define BLOCK_ADDR_SIZE sizeof(long)
#define BLOCK_SIZE 4096

#define DIRECT_NUM 12
#define SINGLE_INDIRECT_NUM 1
#define DOUBLE_INDIRECT_NUM 1
#define TRIPLE_INDIRECT_NUM 1

#define SINGLE_INDIRECT_BLOCK_NUM BLOCK_SIZE / BLOCK_ADDR_SIZE
#define DOUBLE_INDIRECT_BLOCK_NUM SINGLE_INDIRECT_BLOCK_NUM * SINGLE_INDIRECT_BLOCK_NUM
#define TRIPLE_INDIRECT_BLOCK_NUM DOUBLE_INDIRECT_BLOCK_NUM * SINGLE_INDIRECT_BLOCK_NUM


typedef struct Position {
    int block_num;
    int offset;
} Position;

// block_address[15]
static int access_inode_direct_block_num(Inode* inode, int idx, bool is_writed);

static int access_inode_indirect_block_num(int indirect_block_num, int idx, bool is_writed);

/**
 * @param single_indirect_block_num
 * @param ind_block_idx
 * @return
 */
static int through_single_indirect(int single_indirect_block_num, int ind_block_idx, bool is_writed);

/**
 * @param double_indirect_block_num
 * @param ind_block_idx
 * @return
 */
static int through_double_indirect(int double_indirect_block_num, int ind_block_idx, bool is_writed);

/**
 * @param triple_indirect_block_num
 * @param ind_block_idx
 * @return
 */
static int through_triple_indirect(int triple_indirect_block_num, int ind_block_idx, bool is_writed);

/**
 * @param inode
 * @param infile_offset
 * @return
 */
int translate_pos(Inode* inode, Position* position, int infile_offset, bool is_writed = false);
