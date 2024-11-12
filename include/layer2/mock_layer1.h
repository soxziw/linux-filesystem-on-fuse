#include <ctime>
#include <stddef.h>
#include <iostream>


#define ROOT_INODE_NUM 0
#include <algorithm>
#include <iterator>

#define BLOCK_SIZE 4096

//template function to compare arrays
template <typename T, size_t N>
bool array_equal(const T (&a)[N], const T (&b)[N]) {
    return std::equal(std::begin(a), std::end(a), std::begin(b));
}
//superblock should contain inode number for root path
typedef struct SuperBlock {
    time_t creation_ts;
    time_t mount_ts;
    unsigned long i_list_size; //i_list size in blocks
    unsigned long d_block_size; //d_block size in blocks
    unsigned long free_head; //block number of free head (aka the index into the fs)
    int root_inode_num;
    char padding[4052];

    bool operator == (const SuperBlock &sb) {
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
    int file_size;
    //not implementing preferred block size metadata
    int num_allocated_blocks;
    short file_type; //reg file, directory, or special file (device)

    //time_t is a long
    time_t atime; //last access time in unix timestamp
    time_t creation_time; //creation time in unix timestamp
    time_t mtime; //last modification time in unix timestamp
    time_t ctime; //last time file status was modified (e.g. chmod)

    bool operator == (const InodeMData &md) {
        return dev_num == md.dev_num && inode_num == md.inode_num && mode == md.mode && num_hard_links == md.num_hard_links
                && owner_id == md.owner_id && group_id == md.group_id && device_id == md.device_id && file_size == md.file_size
                && num_allocated_blocks == md.num_allocated_blocks && file_type == md.file_type &&
                num_allocated_blocks == md.num_allocated_blocks && file_type == md.file_type && atime == md.atime &&
                creation_time == md.creation_time && mtime == md.mtime && ctime == md.ctime;
    }
} InodeMData;

typedef struct Inode {
    //indices 0-11 are direct data blocks, 12 is single indirect, 13 is double indirect, 14 is triple indirect
    int block_addrs[15]; 
    InodeMData m_data;
    char padding[112]; //padding for inode to be 256 bytes
    bool operator == (const Inode &in) {
        return array_equal(block_addrs, in.block_addrs) && m_data == in.m_data && array_equal(padding, in.padding);
    }
} Inode;

//union so that a block can describe an i-node block, direct block, free data block, indirect block
typedef union Block {
    char dir_block[BLOCK_SIZE];
    unsigned long ind_blocks[BLOCK_SIZE/sizeof(unsigned long)]; //this can be used either as a block in the free list or an indirect block
    Inode inode_block[BLOCK_SIZE/sizeof(Inode)]; //16 inodes per block
    SuperBlock super;

    //possible TODO: write == operator for Block
} Block;

//helper function for mkfs. i_list_size and d_block_size is in blocks
int init_fs(int i_list_size, int d_block_size);

//file type can be either 0, 1, or 2, corresponding to reg file, directory, or special file.
//reurns inode number on success, o.w returns -1
int alloc_inode(const short file_type);

//deallocates inode when ref count = =
int free_inode(const int inode_num);

//returns success/error code, and populates inode with all inode data
int read_inode(const int inode_num, Inode& inode);

//helper function for namei. Not sure if we should implement this in layer 1 or 2
int get_inode_num(const char path[]);

//updates inode blocks[]. Not sure how it'll work when some data blocks in the inode are freed
int write_inode(const int inode_num, const int block_nums[]);

//can be used to unlink a file
int write_inode_mdata(const InodeMData m_data, const int inode_num); 

//returns block number on success, otherwise -1
int alloc_block(); 

//deallocates block/puts back on free list, 0 on sucess, otherwise -1
int free_block(const int block_num);

//data is size 4096; 0 on success, and populates DBlock with data. otherwise returns -1
int read_block(Block& data, const int block_num);

//returns 0 on success, otherwise -1
int write_block(const Block data, const int block_num); 
