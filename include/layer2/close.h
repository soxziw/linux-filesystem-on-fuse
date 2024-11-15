#include <cerrno>
#include "layer1/data_structs.hpp"

/**
 * Close() system call when closing a file in the kernel
 * decrements reference counts and releases buffers as needed (e.g. when ref counts go to zero)
 * @param inode a inode of file to close.
 * @return
 */
int close(Inode* inode);