#include <stddef.h>
#include <cerrno>
#include "mock_layer1.h"

/**
 * write() system call in the kernel
 * Step 1: compute the block index and offset from the r/w pointer
 *      Block index = 0 / 4K
 *      Offset = 0 % 4K
 * Step 2: If block address in i-node is 0 at the Block index
 *      Step 2A: allocate a data block from the free list and record its block address at Block index entry of i-node
 *      Step 2B: allocate memory buffer large enough to hold a data block
 * Step 3: copy data from user buffer to Offset inside memory buffer holding data block
 * Step 4: write i-node back to its location in the i-list
 * Step 5: write memory buffer to data block address contained in Block index entry of i-node
 * @param inode a inode to write to.
 * @param buf a char list to store the content.
 * @param count the size to write.
 * @return
 */
int write(Inode* inode, const char* buf, size_t size, off_t offset);