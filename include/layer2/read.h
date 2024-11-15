#include <stddef.h>
#include <cerrno>
#include "layer1/data_structs.hpp"

/**
 * read() system call in the kernel
 * Step 1: compute the block index and offset from the r/w pointer
 *      Block index = 0 / 4K
 *      Offset = 0 % 4K
 * Step 2: allocate a memory buffer large enough to hold a disk block in the kernel
 * Step 3: read the disk block from the block address contained in the i-node at the Block index into memory buffer
 * Step 4: copy data from Offset into memory buffer into user’s memory buffer specified in system call
 * @param inode a inode to read from.
 * @param buf a char list to store the content.
 * @param count the size to read.
 * @return
 */
int read(Inode* inode, char* buf, size_t size, off_t offset);