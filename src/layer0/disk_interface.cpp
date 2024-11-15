#include <iostream>
#include <cstdlib>
#include <errno.h>
#include "layer0/disk_interface.hpp"

// Move the defination of read_from_block and write_to_block hear in cpp file.
//TODO: could make below functions object oriented. However when we move to actual disk we won't have an in-memory disk we can index into
char in_mem_disk[FILE_SYS_SIZE] = {};
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
    // Pre-decide exactly size to read from, ur previous impl does not work well
    // 1. In "while (i<size && start_byte+i%BLOCK_SIZE!=0)", % has a higher precedence than +. Therefore "(start_byte+i)%BLOCK_SIZE!=0" is the right way.
    // 2. Even though "(start_byte+i)%BLOCK_SIZE!=0" perform wrong when start_byte is k * BLOCK_SIZE (also known as offset is 0), u r unable to read anything.
    // 3. Even though "(start_byte+i+1)%BLOCK_SIZE!=0" perform wrong when start_byte is k * BLOCK_SIZE and size is BLOCK_SIZE, u ll return BLOCK_SIZE - 1 (supposed to be BLOCK_SIZE).
    size = std::min(size_t(BLOCK_SIZE - offset), size); 
    int i = 0;
    while (i<size){
        dest_buf[i] = in_mem_disk[start_byte+i];
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
int write_to_block(const char* src_buf, size_t block, unsigned short offset, size_t size){
    //EFBIG if writing past 
    size_t start_byte = BLOCK_SIZE*block+offset;
    // Pre-decide exactly size to write to, ur previous impl does not work well
    // 1. In "while (i<size && start_byte+i%BLOCK_SIZE!=0)", % has a higher precedence than +. Therefore "(start_byte+i)%BLOCK_SIZE!=0" is the right way.
    // 2. Even though "(start_byte+i)%BLOCK_SIZE!=0" perform wrong when start_byte is k * BLOCK_SIZE (also known as offset is 0), u r unable to write anything.
    // 3. Even though "(start_byte+i+1)%BLOCK_SIZE!=0" perform wrong when start_byte is k * BLOCK_SIZE and size is BLOCK_SIZE, u ll return BLOCK_SIZE - 1 (supposed to be BLOCK_SIZE).
    size = std::min(size_t(BLOCK_SIZE - offset), size);

    int i = 0;
    while (i<size){
        in_mem_disk[start_byte+i] = src_buf[i];
        i++;
    }
    return i;
}