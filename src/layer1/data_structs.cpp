#include "layer1/data_structs.hpp"
#include "layer0/disk_interface.hpp"
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <assert.h>
//TODO: includes for layer 0 api

/*
steps:
1) Initialize superblock, write to disk
2) Initialize i-list, write to disk
3) Initialize free list, write to disk
*/

//might have to allocate blocks on the heap due to stack size
int init_fs(long i_list_size, long d_block_size) {
    //When assigning block addresses (i.e. free list head, ind blocks), an address that's invalid/out of range
    //will be assigned -1, which indicates a null ptr.

    if (i_list_size + d_block_size + 1 > PARTITION_SIZE || (i_list_size < 0 || d_block_size < 0))
        return -1;

    //step 1: initialize superblock, write to disk
    Block sup;
    sup.super = { 
        //SETTING CREATION_TS AND MOUNT_TS FOR UNIT TESTING. CHANGE BACK TO time(NULL)
        .creation_ts = 1,
        .mount_ts = 1,
        .i_list_size = (unsigned long)i_list_size,
        .d_block_size = (unsigned long)d_block_size,
        .free_head = d_block_size > 0 ? i_list_size + 1 : -1,
        .root_inode_num = ROOT_INODE_NUM,
        .padding = {0} //padding to make super block 4k bytes
    };
    write_to_block((const char*)&sup, 0, 0, BLOCK_SIZE); //this is a layer 0 function

    //step 2: initialize i-list, write to disk
    Block* i_list = new Block[i_list_size];
    for (int i = 0; i < i_list_size; i++) {
        for (int j = 0; j < BLOCK_SIZE/sizeof(Inode); j++) {
            i_list[i].inode_block[j] = {
                .block_addrs = {0},
                .m_data = {
                    .dev_num = -1,
                    .inode_num = i * (BLOCK_SIZE/sizeof(Inode)) + j, //first inode num starts at 0
                    .mode = -1,
                    .num_hard_links = -1,
                    .owner_id = -1,
                    .group_id = -1,
                    .device_id = -1,
                    .file_size = -1,
                    .num_allocated_blocks = 0,
                    .file_type = 1,
                    .is_allocated = ((short int)((i * (BLOCK_SIZE/sizeof(Inode)) + j) == 0) ? true : false), //root inode num is allocated upon mkfs
                    .atime = -1,
                    .creation_time = -1,
                    .mtime = -1,
                    .ctime = -1,
                },
                .padding = {0}
            };
        }
        write_to_block((const char*)&i_list[i], i + 1, 0, BLOCK_SIZE);
    }
    delete[] i_list;

    //step 3: initialize free list, write to disk
    //implement the SysV free list (each data block holds 512 block addresses)
    int block_nums_per_block = BLOCK_SIZE/sizeof(unsigned long); //512
    int free_block_num = sup.super.free_head; //the block number corresponding to a free node in the free list
    for (int i = 0; i < d_block_size; i += block_nums_per_block) {
        Block free_node; //a free node in the free list
        //if free_node tries to add a block address that's out of range, it will set the address to -1
        //this indicates that the index is neither allocated nor a valid block addr
        int next_free_block_num = free_block_num + block_nums_per_block + 1;
        free_node.ind_blocks[0] = next_free_block_num < PARTITION_SIZE ? next_free_block_num : -1;
        for (int j = 1; j < block_nums_per_block; j++) {
            if (free_block_num + j < PARTITION_SIZE)
                free_node.ind_blocks[j] = free_block_num + j;
            else
                free_node.ind_blocks[j] = -1;
        }
        write_to_block((const char*)&free_node, free_block_num, 0, BLOCK_SIZE);
        free_block_num = free_node.ind_blocks[0];
        if (free_block_num == -1) //this means that there are no more data blocks in the partition that can be marked free
            break;
    }
    return 0;
}

int alloc_inode(const short file_type) {
    if (file_type < 0 || file_type > 2)
        return -1;

    //linearly scan i-list and find i-node that is free/unallocated (is_allocated = 0)
    //first need to read from super block to get i-list size
    Block sup;
    int res = read_from_block((char*)&sup, 0, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    bool found_free_inode = false;
    int inode_block_ind = 1;
    int inode_ret = -2; //the returned inode number. Defining -2 as the case where all inodes are allocated
    Block inode_block;
    while (inode_block_ind < sup.super.i_list_size + 1 && !found_free_inode) {
        res = read_from_block((char*)&inode_block, inode_block_ind, 0, BLOCK_SIZE);
        if (res != BLOCK_SIZE)
            return res;
        int j = 0;
        while (!found_free_inode && j < BLOCK_SIZE/sizeof(Inode)) {
            InodeMData inode_md = inode_block.inode_block[j].m_data;
            if (!inode_md.is_allocated) {
                inode_block.inode_block[j].m_data.is_allocated = true;
                inode_block.inode_block[j].m_data.file_type = file_type;
                inode_ret = inode_md.inode_num;
                found_free_inode = true;
            }
            j++;
        }
        if (found_free_inode) break;
        inode_block_ind++;
    }
    if (found_free_inode) {
        res = write_to_block((const char*)&inode_block, inode_block_ind, 0, BLOCK_SIZE);
        if (res != BLOCK_SIZE)
            return res;
    }
        
    return inode_ret; //this will either be -2 (failure) or the inode number
        
}

int free_inode(const int inode_num) {
    //check if inode out of range
    Block sup;
    int res = read_from_block((char*)&sup, 0, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    if (inode_num >= (BLOCK_SIZE/sizeof(Inode) * sup.super.i_list_size) || inode_num <= 0)
        return -3; 
    int block_addr = inode_num / (BLOCK_SIZE / sizeof(Inode)) + 1;
    int inode_block_ind = inode_num % (BLOCK_SIZE / sizeof(Inode));
    Block inode_block;
    res = read_from_block((char*)&inode_block, block_addr, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    //clear block addresses in inode w/ inode number = inode_num
    Inode* freed_inode = &inode_block.inode_block[inode_block_ind];
    for (int i = 0; i < INODE_BLOCKS; i++) {
        long curr_block_addr = freed_inode->block_addrs[i];
        switch (i) {
            case 14:
                res = free_t_ind_block(curr_block_addr);
                if (res != 0)
                    return res;
                break;
            case 13:
                res = free_d_ind_block(curr_block_addr);
                if (res != 0)
                    return res;
                break;
            case 12:
                res = free_ind_block(curr_block_addr);
                if (res != 0)
                    return res;
                break;
            default:
                res = free_block(curr_block_addr);
                if (res != 0)
                    return res;
                break;
        }
        freed_inode->block_addrs[i] = 0; // Fix error default value, u use 0 as default in init_fs and alloc_inode.
    }

    //reset inode metadata
    freed_inode->m_data = {
        .dev_num = -1,
        .inode_num = (unsigned long) inode_num, //retain inode number
        .mode = -1,
        .num_hard_links = -1,
        .owner_id = -1,
        .group_id = -1,
        .device_id = -1,
        .file_size = -1,
        .num_allocated_blocks = 0,
        .file_type = -1,
        .is_allocated = 0,
        .atime = -1,
        .creation_time = -1,
        .mtime = -1,
        .ctime = -1,
    };
    res = write_to_block((const char*)&inode_block, block_addr, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    return 0;
}

int read_inode(const int inode_num, Inode& inode) {
    //check if inode out of range
    Block sup;
    int res = read_from_block((char*)&sup, 0, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    if (inode_num >= (BLOCK_SIZE/sizeof(Inode) * sup.super.i_list_size) || inode_num < 0)
        return -3; 
    int block_addr = inode_num / (BLOCK_SIZE / sizeof(Inode)) + 1;
    int inode_block_ind = inode_num % (BLOCK_SIZE / sizeof(Inode));
    Block inode_block;
    res = read_from_block((char*)&inode_block, block_addr, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    if (!inode_block.inode_block[inode_block_ind].m_data.is_allocated) { // Add check to inode validity.
        return -4;
    }
    inode = inode_block.inode_block[inode_block_ind];
    return 0;
}

// int write_inode(const int inode_num, const int block_nums[], const int n) {
//     if (n != 15)
//         return -1;
//     //check if inode out of range
//     Block sup;
//     int res = read_block(0, sup);
//     if (res != BLOCK_SIZE)
//         return res;
//     if (inode_num >= (BLOCK_SIZE/sizeof(Inode) * sup.super.i_list_size) || inode_num <= 0)
//         return -3; 
//     int block_addr = inode_num / (BLOCK_SIZE / sizeof(Inode)) + 1;
//     int inode_block_ind = inode_num % (BLOCK_SIZE / sizeof(Inode));
//     Block inode_block;
//     res = read_block(block_addr, inode_block);
//     if (res != BLOCK_SIZE)
//         return res;

//     Inode* written_inode = &inode_block.inode_block[inode_block_ind];
//     int i = 0;
//     for (auto& addr : written_inode->block_addrs) {
//         addr = block_nums[i];
//         i++;
//     }
        
//     res = write_block(block_addr, inode_block);
//     if (res != BLOCK_SIZE)
//         return res;
//     return 0;
// }

int write_inode(const int inode_num, const long block_num, const int inode_ind) {
    if (inode_ind < 0 || inode_ind > 14)
        return -1;
    Block sup;
    int res = read_from_block((char*)&sup, 0, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    //check if inode_num is out of range
    if (inode_num >= (BLOCK_SIZE/sizeof(Inode) * sup.super.i_list_size) || inode_num < 0)
        return -3; 

    int block_addr = inode_num / (BLOCK_SIZE / sizeof(Inode)) + 1;
    int inode_block_ind = inode_num % (BLOCK_SIZE / sizeof(Inode));
    Block inode_block;
    res = read_from_block((char*)&inode_block, block_addr, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    Inode* written_inode = &inode_block.inode_block[inode_block_ind];
    written_inode->block_addrs[inode_ind] = block_num;
        
    res = write_to_block((const char*)&inode_block, block_addr, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    return 0;
}

int write_inode_mdata(const InodeMData m_data, const int inode_num) {
    Block sup;
    int res = read_from_block((char*)&sup, 0, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    //check if inode_num is out of range
    if (inode_num >= (BLOCK_SIZE/sizeof(Inode) * sup.super.i_list_size) || inode_num < 0)
        return -3; 
    int block_addr = inode_num / (BLOCK_SIZE / sizeof(Inode)) + 1;
    int inode_block_ind = inode_num % (BLOCK_SIZE / sizeof(Inode));
    Block inode_block;
    res = read_from_block((char*)&inode_block, block_addr, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    Inode* written_inode = &inode_block.inode_block[inode_block_ind];
    written_inode->m_data = m_data;
        
    res = write_to_block((const char*)&inode_block, block_addr, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    return 0;
}

//TODO: update metadata after freeing block
//deletes block_num that is present in inode_num. Could be direct, single indirect, double indirect
//for now, don't test this as we'll handle truncate later
extern int delete_from_inode(const int inode_num, const long target_block_addr) {
    return 0;
}

long alloc_block() {
    int res;
    Block sup;
    res = read_from_block((char*)&sup, 0, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    long free_head = sup.super.free_head;
    //check if free list is empty. If so return error
    if (free_head == -1) // Fix typo of use "="
        return -1;
    
    bool found_free_block = false;
    long block_addr;
    Block free_node;
    res = read_from_block((char*)&free_node, free_head, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    int i = 1;
    while (!found_free_block && i < (BLOCK_SIZE / sizeof(long))) {
        long curr_block_addr = free_node.ind_blocks[i];
        if (curr_block_addr != 0) {
            found_free_block = true;
            block_addr = curr_block_addr;
            free_node.ind_blocks[i] = 0;
            write_to_block((const char*)&free_node, free_head, 0, BLOCK_SIZE);
        }
        i++;
    }
    if (!found_free_block) {
        block_addr = free_head;
        sup.super.free_head = free_node.ind_blocks[0];
        write_to_block((const char*)&sup, 0, 0, BLOCK_SIZE);
    }
    return block_addr;
}

//TODO: update metadata after freeing block
int free_block(const int block_num) {
    //start at head of free list, and check for a zero'd out entry. If one exists, fill it with block we're freeing
    //otherwise if the head of the freelist contains all nonzero entries, set the head of the free list to point
    //to the freed block. Have the freed block's next ptr point to the old head
    if (block_num == 0) { // Add check for unallocated block, no need to free that.
        return 0;
    }
    Block sup;
    int res = read_from_block((char*)&sup, 0, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    long free_head = sup.super.free_head;
    //check if free list is empty. If so return error
    if (free_head == -1)
        return -1;
    
    Block free_head_node;
    res = read_from_block((char*)&free_head_node, free_head, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    bool found_zero = false;
    int i = 1;
    while (!found_zero && i < (BLOCK_SIZE / sizeof(long))) {
        if (free_head_node.ind_blocks[i] == 0) {
            found_zero = true;
            free_head_node.ind_blocks[i] = block_num;
        }
        i++;
    }

    if (found_zero) {
        write_to_block((const char*)&free_head_node, free_head, 0, BLOCK_SIZE);
    } else {
        //have freed block be new head
        Block new_free_head_node;
        new_free_head_node.ind_blocks[0] = free_head;
        for (int i = 1; i < (BLOCK_SIZE / sizeof(long)); i++) {
            new_free_head_node.ind_blocks[i] = 0;
        }
        sup.super.free_head = block_num;
        write_to_block((const char*)&new_free_head_node, block_num, 0, BLOCK_SIZE);
        write_to_block((const char*)&sup, 0, 0, BLOCK_SIZE);
    }
    return 0;
}

int free_ind_block(const long block_num) {
    if (block_num == 0) { // Add check for unallocated block, no need to free that.
        return 0;
    }
    Block ind_block;
    int res = read_from_block((char*)&ind_block, block_num, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    for (auto& addr : ind_block.ind_blocks) {
        res = free_block(addr);
        if (res != BLOCK_SIZE)
            return res;
        addr = -1;
    }

    free_block(block_num);
    res = write_to_block((const char*)&ind_block, block_num, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    return 0;
}

int free_d_ind_block(const long block_num) {
    if (block_num == 0) {
        return 0;
    }
    Block ind_block;
    int res = read_from_block((char*)&ind_block, block_num, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    for (auto& addr : ind_block.ind_blocks)
        free_ind_block(addr);

    free_block(block_num);
    res = write_to_block((const char*)&ind_block, block_num, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    return 0;
}

int free_t_ind_block(const long block_num) {
    if (block_num == 0) { // Add check for unallocated block, no need to free that.
        return 0;
    }
    Block ind_block;
    int res = read_from_block((char*)&ind_block, block_num, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    for (auto& addr : ind_block.ind_blocks)
        free_d_ind_block(addr);

    free_block(block_num);
    res = write_to_block((const char*)&ind_block, block_num, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;
    return 0;
}

int read_free_list(std::vector<Block>& f_list) {
    Block super;
    int res = read_from_block((char*)&super, 0, 0, BLOCK_SIZE);
    if (res != BLOCK_SIZE)
        return res;

    //block addr of current free node
    unsigned long curr_free_node_addr = super.super.free_head;
    //check if free list is empty. If it's empty, the free_head will be -1
    if (curr_free_node_addr == -1) {
        return 0;
    }

    //append each free node onto f_list, and break once index 0 of the free node no longer points to a valid block addr
    while (curr_free_node_addr < PARTITION_SIZE) {
        Block free_node;
        res = read_from_block((char*)&free_node, curr_free_node_addr, 0, BLOCK_SIZE);
        if (res != BLOCK_SIZE)
            return res;
        f_list.push_back(free_node);
        curr_free_node_addr = free_node.ind_blocks[0];
        if (curr_free_node_addr == -1)
            break;
    }

    return 0;
}

void print_free_list(std::vector<Block>& f_list) {
    for (auto& b : f_list) {
        std::cout << "[";
        for (auto& block_num : b.ind_blocks)
            std::cout << block_num << ",";
        std::cout << "]\n";
    }
}