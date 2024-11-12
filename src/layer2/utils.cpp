#include "layer2/utils.h"

int access_inode_direct_block_num(Inode* inode, int idx, bool is_write) {
    if (inode->block_addrs[idx] == 0 && is_write) {
        int new_block_num = alloc_block();
        if (new_block_num != -1) {
            inode->block_addrs[idx] = new_block_num;
            write_inode(inode->m_data.inode_num, inode->block_addrs);
        }
    }
    return inode->block_addrs[idx];
}

int access_inode_indirect_block_num(int indirect_block_num, int idx, bool is_write) {
    Block data;
    if (read_block(data, indirect_block_num)) {
        std::cerr << "Error: touch indirect block error. " 
                    << "indirect_block_num: " << indirect_block_num << "." << std::endl;
        return 0;
    }
    if (data.ind_blocks[idx] == 0 && is_write) {
        int new_block_num = alloc_block();
        if (new_block_num != -1) {
            data.ind_blocks[idx] = new_block_num;
            write_block(data, indirect_block_num);
        }
    }
    return data.ind_blocks[idx];
}

int through_single_indirect(int single_indirect_block_num, int ind_block_idx, bool is_write) {
    int block_num;
    if ((block_num = access_inode_indirect_block_num(single_indirect_block_num, ind_block_idx, is_write)) != 0 ) {
        return block_num;
    }
    return 0;
}

int through_double_indirect(int double_indirect_block_num, int ind_block_idx, bool is_write) {
    int single_indirect_block_num;
    if ((single_indirect_block_num = access_inode_indirect_block_num(double_indirect_block_num, ind_block_idx, is_write)) != 0) {
        return through_single_indirect(single_indirect_block_num, ind_block_idx % SINGLE_INDIRECT_BLOCK_NUM, is_write); 
    }
    return 0;
}

int through_triple_indirect(int triple_indirect_block_num, int ind_block_idx, bool is_write) {
    int double_indirect_block_num;
    if ((double_indirect_block_num = access_inode_indirect_block_num(triple_indirect_block_num, ind_block_idx, is_write)) != 0) {
        return through_double_indirect(double_indirect_block_num, ind_block_idx % DOUBLE_INDIRECT_BLOCK_NUM, is_write);
    }
    return 0;
}

int translate_pos(Inode* inode, Position* position, int infile_offset, bool is_write) {
    int infile_block_num = infile_offset / BLOCK_SIZE;
    if (infile_block_num < DIRECT_NUM) { // direct data block
        position->block_num = access_inode_direct_block_num(inode, infile_block_num, is_write);
    } else if ((infile_block_num -= DIRECT_NUM) 
                    < SINGLE_INDIRECT_BLOCK_NUM * SINGLE_INDIRECT_NUM) { // single direct data block
        int single_indirect_block_num;
        if ((single_indirect_block_num = access_inode_direct_block_num(inode, DIRECT_NUM + infile_block_num / SINGLE_INDIRECT_BLOCK_NUM, is_write)) != 0) {
            position->block_num = through_single_indirect(single_indirect_block_num, infile_block_num % SINGLE_INDIRECT_BLOCK_NUM, is_write);
        }
    } else if ((infile_block_num -= SINGLE_INDIRECT_BLOCK_NUM * SINGLE_INDIRECT_NUM)
                    < DOUBLE_INDIRECT_BLOCK_NUM * DOUBLE_INDIRECT_NUM) { // double direct data block
        int double_indirect_block_num;
        if ((double_indirect_block_num = access_inode_direct_block_num(inode, DIRECT_NUM + SINGLE_INDIRECT_NUM + infile_block_num / DOUBLE_INDIRECT_BLOCK_NUM, is_write)) != 0) {
            position->block_num = through_double_indirect(double_indirect_block_num, infile_block_num % DOUBLE_INDIRECT_BLOCK_NUM, is_write);
        }
    } else if ((infile_block_num -= DOUBLE_INDIRECT_BLOCK_NUM * DOUBLE_INDIRECT_NUM)
                    < TRIPLE_INDIRECT_BLOCK_NUM * TRIPLE_INDIRECT_NUM) { // triple direct data block
        int triple_indirect_block_num;
        if ((triple_indirect_block_num = access_inode_direct_block_num(inode, DIRECT_NUM + SINGLE_INDIRECT_NUM + DOUBLE_INDIRECT_NUM + infile_block_num / TRIPLE_INDIRECT_BLOCK_NUM, is_write)) != 0) { 
            position->block_num = through_triple_indirect(triple_indirect_block_num, infile_block_num % TRIPLE_INDIRECT_BLOCK_NUM, is_write);
        }
    } else {
        position->block_num = 0;
    }
    position->offset = infile_offset % BLOCK_SIZE;
    if (position->block_num == 0) {
        return -1;
    } else {
        return 0;
    } 
}
