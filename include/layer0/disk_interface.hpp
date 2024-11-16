#ifndef DISK_INTERFACE
#define DISK_INTERFACE

#include <iostream>
#include <cstdlib>
#include <errno.h>
#include <array>
#include "layer1/data_structs.hpp"

#define PARTITION_SIZE 1000 //partition size in blocks
#define FILE_SYS_SIZE PARTITION_SIZE * BLOCK_SIZE

extern char in_mem_disk[FILE_SYS_SIZE]; 

/*
params:
buf - buffer to populate with data from block(s)
block - logical block # of the block to start reading at
offset - the byte offset within the block to begin reading at
size - read up to size bytes beginning at offset
returns: number of bytes successfully read, zero if EOF, or error
*/
int read_from_block(char* dest_buf, size_t block, unsigned short offset, size_t size); 

/*
params:
buf - buffer to write data from into the block
block - logical block # of the block to start writing at
offset - the byte offset within the block to begin writing at
size - write up to size bytes beginning at offset
returns: number of bytes successfully read, or error
*/
int write_to_block(const char* src_buf, size_t block, unsigned short offset, size_t size);

#endif