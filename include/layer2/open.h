#include <fuse.h>
#include <cerrno>
#include "mock_layer1.h"


/**
 * Open() system call when creating a file in the kernel
 * Step 1: Allocate a free i-node
 *      Step 1A: allocate a buffer large enough to hold a disk block in kernel memory
 *      Step 1B: read the first block of the i-list into that memory buffer
 *      Step 1C: examine the allocate flag of each i-node in the disk block If allocate flag clear, copy i-node into memory buffer for i-node and note the inode number
 *      Step 1D: if no free i-node in this block, repeat 1B with next block in i-list
 * Step 2: record permissions, creation time, and file owner in i-node in memory buffer
 * Step 3: write the block containing the i-node back to disk
 * @param path a const char list of the path to open.
 * @param flags a int contains flag bits.
 * @return 
 */
int open(Inode* inode, int flags, fuse_context* context);