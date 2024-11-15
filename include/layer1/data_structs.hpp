#ifndef FILESYSTEM
#define FILESYSTEM

#include <ctime>
#include <cstdint> 
#include <string.h>
#include <vector>
#include "utils.hpp"

#define ROOT_INODE_NUM 0

//superblock should contain inode number for root path
typedef struct SuperBlock {
    time_t creation_ts;
    time_t mount_ts;
    unsigned long i_list_size; //i_list size in blocks
    unsigned long d_block_size; //d_block size in blocks
    long free_head; //block number of free head (aka the index into the fs)
    int root_inode_num;
    char padding[4052];

    bool operator == (const SuperBlock& sb) {
        return creation_ts == sb.creation_ts && mount_ts == sb.mount_ts && i_list_size == sb.i_list_size &&
                d_block_size == sb.d_block_size && free_head == sb.free_head && root_inode_num == sb.root_inode_num
                && array_equal(padding, sb.padding);
    } 
} SuperBlock;

typedef struct InodeMData { //size (padded): 72 bytes
    int dev_num;
    unsigned long inode_num;
    int mode; //file permissions (9-digit octal number); 27 bits of storage
    int num_hard_links;
    int owner_id; //user id of user who created this file/inode
    int group_id; //user id of group owner (parent directory)
    int device_id; //used for files that are devices
    long file_size;
    int num_allocated_blocks;
    short file_type; //reg file, directory, or special file (device)
    bool is_allocated;

    //time_t is a long
    time_t atime; //last access time in unix timestamp
    time_t creation_time; //creation time in unix timestamp
    time_t mtime; //last modification time in unix timestamp
    time_t ctime; //last time file status was modified (e.g. chmod)

    bool operator == (const InodeMData& md) {
        return dev_num == md.dev_num && inode_num == md.inode_num && mode == md.mode && num_hard_links == md.num_hard_links
                && owner_id == md.owner_id && group_id == md.group_id && device_id == md.device_id && file_size == md.file_size
                && num_allocated_blocks == md.num_allocated_blocks && file_type == md.file_type &&
                num_allocated_blocks == md.num_allocated_blocks && file_type == md.file_type && atime == md.atime &&
                creation_time == md.creation_time && mtime == md.mtime && ctime == md.ctime;
    }
} InodeMData;

typedef struct Inode {
    //indices 0-11 are direct data blocks, 12 is single indirect, 13 is double indirect, 14 is triple indirect
    long block_addrs[15]; //might need to make these of type long
    InodeMData m_data; //TODO: make sure padding is correct
    char padding[48]; //padding for inode to be 256 bytes
    bool operator == (const Inode& in) {
        return array_equal(block_addrs, in.block_addrs) && m_data == in.m_data && array_equal(padding, in.padding);
    }
} Inode;

//union so that a block can describe an i-node block, direct block, free data block, indirect block
typedef union Block {
    char dir_block[BLOCK_SIZE];
    // DirEntry dir_entries[BLOCK_SIZE/sizeof(DirEntry)]; For Yuchen's mkdir
    long ind_blocks[BLOCK_SIZE/sizeof(unsigned long)]; //this can be used either as a block in the free list or an indirect block
    Inode inode_block[BLOCK_SIZE/sizeof(Inode)]; //16 inodes per block
    SuperBlock super;
    //possible TODO: write == operator for Block
} Block;

//helper function for mkfs. i_list_size and d_block_size is in blocks
extern int init_fs(long i_list_size, long d_block_size);

//file type can be either 0, 1, or 2, corresponding to reg file, directory, or special file.
//reurns inode number on success, o.w returns -1
extern int alloc_inode(const short file_type);

//deallocates inode when ref count = =
extern int free_inode(const int inode_num);

//on success, populates inode with all inode data and returns 0. Otherwise returns negative number
extern int read_inode(const int inode_num, Inode& inode);

//updates inode blocks[]. n is the size of block_nums (should be 15). Not sure how it'll work when some data blocks in the inode are freed
//this is probably a useless function. write_data_block and append_data_block are likely more useful
// extern int write_inode(const int inode_num, const int block_nums[], const int n);

//writes a single data block to an inode at a given index. Returns 0 on success, otherwise -1
//TODO: update Inode M_data file size field
extern int write_inode(const int inode_num, const long block_num, const int inode_idx);

//can be used to unlink a file
extern int write_inode_mdata(const InodeMData m_data, const int inode_num); 

//deletes block_num that is present in inode_num. Could be direct, single indirect, double indirect
//Not testing for this now, as we're not worrying about truncate for now
extern int delete_from_inode(const int inode_num, const long target_block_addr);

//returns block number on success, and removes block address from the free list
extern long alloc_block(); 

//deallocates block/puts back on free list, 0 on sucess, otherwise -1
extern int free_block(const int block_num);

//recursively deletes all blocks inside indirect block, then frees indirect block.
//Returns 0 on success, otherwise negative no.
extern int free_ind_block(const long block_num);

//recursively deletes all blocks inside double indirect block (calling free_ind_block), then frees indirect block.
//Returns 0 on success, otherwise negative no.
extern int free_d_ind_block(const long block_num);

//recursively deletes all blocks inside double indirect block (calling free_ind_block), then frees indirect block.
//Returns 0 on success, otherwise negative no.
extern int free_t_ind_block(const long block_num);

//returns 0 on sucess and populates f_list with file system's free list, otherwise -1
extern int read_free_list(std::vector<Block>& f_list);

//prints free list
extern void print_free_list(std::vector<Block>& f_list);

#endif