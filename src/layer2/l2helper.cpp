#include "layer2/l2helper.h"
#include "layer2/translate_pos.h"
#include "layer0/disk_interface.hpp"
std::vector<std::string> tokenize_path(const std::string& path) {
    std::vector<std::string> components;
    size_t pos = 0;
    size_t len = path.length();

    while (pos < len) {
        // Skip multiple slashes
        while (pos < len && path[pos] == '/') {
            pos++;
        }

        if (pos == len) {
            break;
        }

        // Find the next slash or end of string
        size_t next_slash = path.find('/', pos);
        if (next_slash == std::string::npos) {
            next_slash = len;
        }

        // Extract the component
        std::string component = path.substr(pos, next_slash - pos);
        components.push_back(component);

        pos = next_slash;
    }

    return components;
}

int read_directory(Inode* dir_inode, std::vector<DirEntry>& entries) {
    if (dir_inode->m_data.file_type != 1) {
        return -ENOTDIR; // Not a directory
    }

    int dir_size = dir_inode->m_data.file_size; // Size of the directory in bytes
    int offset = 0;

    while (offset < dir_size) {
        Position position;
        //get a block number
        int res = translate_pos(dir_inode, &position, offset, false); // is_write = false
        if (res != 0) {
            return res; // Error translating position
        }

        //readfromblock to get the each directory from this dir inode
        DirEntry entry;
        res = read_from_block((char*)(&entry), position.block_num,position.offset,DIRENTRY_SIZE);
        if (res != 0) {
            return res; // Error reading block
        }

        // Check for valid entry
        if (entry.inode_num != 0) {
            entries.push_back(entry);
        }

        offset += sizeof(DirEntry);
    }

    return 0; // Success
}

int namei(const std::string& path, int& inode_num) {
    // Validate that the path is absolute
    if (path.empty() || path[0] != '/') {
        return -EINVAL; // Invalid argument: path must start with '/'
    }

    // Tokenize the path into components
    std::vector<std::string> components = tokenize_path(path);

    // Start from the root inode
    int current_inode_num = ROOT_INODE_NUM;
    Inode current_inode;

    // Read the root inode
    int res = read_inode(current_inode_num, current_inode);
    if (res != 0) {
        return res; // Error reading root inode
    }

    int num_components = components.size();
    // if (num_components == 0)
    // {
    //     inode_num=ROOT_INODE_NUM;
    //     return 0;
    // }
    
    // Iterate over each component in the path
    for (int i = 0; i < num_components-1; ++i) {
        const auto& component = components[i];

        if (component.empty() || component == ".") {
            // Skip empty or current directory components
            continue;
        } else if (component == "..") {
            // Handle the parent directory component
            if (current_inode_num == ROOT_INODE_NUM) {
                // Already at root, cannot go up further
                continue;
            }

            // Ensure the current inode is a directory before reading entries
            if (current_inode.m_data.file_type != 1) { // Not a directory
                return -ENOTDIR;
            }

            // Read the '..' entry to get the parent inode number
            std::vector<DirEntry> entries;
            res = read_directory(&current_inode, entries);
            if (res != 0) {
                return res;
            }

            bool found_parent = false;
            for (const auto& entry : entries) {
                if (strcmp(entry.name, "..") == 0) {
                    current_inode_num = entry.inode_num;

                    // Read the parent inode
                    res = read_inode(current_inode_num, current_inode);
                    if (res != 0) {
                        return res;
                    }

                    found_parent = true;
                    break;
                }
            }

            // if (!found_parent) {
            //     // '..' entry not found, assume root
            //     current_inode_num = ROOT_INODE_NUM;
            //     res = read_inode(current_inode_num, current_inode);
            //     if (res != 0) {
            //         return res;
            //     }
            // }
            // continue;
        }

        // // Before attempting to read directory entries, ensure current inode is a directory
        // if (current_inode.m_data.file_type != 1) { // Not a directory
        //     return -ENOTDIR; // Cannot read entries from a non-directory
        // }

        // Read the directory entries of the current inode
        std::vector<DirEntry> entries;
        res = read_directory(&current_inode, entries);
        if (res != 0) {
            return res;
        }

        // Search for the component in the directory entries
        bool found = false;
        for (const auto& entry : entries) {
            if (strcmp(entry.name, component.c_str()) == 0) {
                // Found the component; update current inode
                current_inode_num = entry.inode_num;

                // Read the inode of the found entry
                res = read_inode(current_inode_num, current_inode);
                if (res != 0) {
                    return res;
                }

                found = true;
                break;
            }
        }

        if (!found) {
            // Component not found in current directory
            return -ENOENT; // No such file or directory
        }
    }

    inode_num = current_inode_num;
    return 0; // Success
}
