#ifndef DISK_INTERFACE
#define DISK_INTERFACE

#include <iostream>
#include <cstdlib>
#include <errno.h>
#include "utils.hpp"
#include "layer1/data_structs.hpp"

#define PARTITION_SIZE 9 //partition size in blocks

//TODO: could make below functions object oriented. However when we move to actual disk we won't have an in-memory disk we can index into
extern Block file_partition[PARTITION_SIZE]; //the simulated disk

//takes in block_num (for now index into file_partition[]) and writes const Block block into file_partition[]
//returns 0 upon success, -1 o.w
extern int write_block(const long block_num, const Block block);

//takes in block_num (for now index into file_partition[]) and writes const Block block into file_partition[]
//returns 0 upon success and populates Block& block, -1 o.w
extern int read_block(const long block_num, Block& block);

class disk_interface {
    public:

        disk_interface(){

        }
        /*
        params:
        buf - buffer to populate with data from block(s)
        block - logical block # of the block to start reading at
        offset - the byte offset within the block to begin reading at
        size - read up to size bytes beginning at offset
        returns: number of bytes successfully read, zero if EOF, or error
        */
        int read_from_block(char* dest_buf, size_t block, unsigned short offset, size_t size){
            //1. get block index
            //2. get block offset
            //3. read data into buf
            //4. 
            size_t start_byte = BLOCK_SIZE*block + offset;
            int i = 0;
            while (i<size && start_byte+i%BLOCK_SIZE!=0){
                dest_buf[i] = file_partition->dir_block[start_byte+i];
                i++;
            }
            return i;
        }

        /*
        params:
        buf - buffer to write data from into the block
        block - logical block # of the block to start writing at
        offset - the byte offset within the block to begin writing at
        size - write up to size bytes beginning at offset
        returns: number of bytes successfully read, or error
        */
        int write_to_block(char* src_buf, size_t block, unsigned short offset, size_t size){
            //EFBIG if writing past 
            size_t start_byte = BLOCK_SIZE*block+offset;
            int i = 0;
            while (i<size && start_byte+i%BLOCK_SIZE!=0){
                file_partition->dir_block[start_byte+i] = src_buf[i];
                i++;
            }
            return i;
        }
	
    private:
        Block temp_disk[10];
};

#endif