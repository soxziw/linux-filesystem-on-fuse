#include <iostream>
#include <cstdlib>
#include <errno.h>
#include "utils.hpp"
#include "layer0/disk_interface.hpp"

Block file_partition[PARTITION_SIZE] = {0};

//TODO: figure out errno codes
int write_block(const long block_num, const Block block) {
    if (block_num >= PARTITION_SIZE || block_num < 0) return -1;
    file_partition[block_num] = block;
    return 0;
}

//TODO: figure out errno codes
int read_block(const long block_num, Block& block) {
    if (block_num >= PARTITION_SIZE || block_num < 0) return -1;
    block = file_partition[block_num];
    return 0;
}
