#include "layer2/readdir.h"

int readdir(const std::string& path, std::vector<DirEntry>& entries) {
    // Validate the path
    if (path.empty() || path[0] != '/') {
        return -EINVAL; // Invalid path
    }

    // Resolve directory inode number
    int dir_inode_num;
    int res = namei(path, dir_inode_num);
    if (res != 0) {
        return res; // Directory not found
    }

    // Read directory inode
    Inode dir_inode;
    res = read_inode(dir_inode_num, dir_inode);
    if (res != 0) {
        return res;
    }

    // Ensure it's a directory
    if (dir_inode.m_data.file_type != 1) {
        return -ENOTDIR;
    }

    // Read directory entries
    res = read_directory(&dir_inode, entries);
    if (res != 0) {
        return res;
    }

    return 0; // Success
}
