#ifndef L2HLEPER_H
#define L2HLEPER_H

#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <cstring>   // For strcmp
#include <cerrno>    // For standard error codes like ENOTDIR
#include <sys/stat.h> // For S_IFDIR macro
#include <sys/types.h>
#include <unistd.h>
#include "layer1/data_structs.hpp"

// Maximum length of a filename
#define NAME_MAX 115
#define DIRENTRY_SIZE 128
// Directory Entry Structure
struct DirEntry {
    uint32_t inode_num;       // Inode number of the entry
    char name[NAME_MAX + 1];  // Null-terminated filename
    char padding[8];
    // padding can be divided by 4096
    // Constructor for convenience
    DirEntry() : inode_num(0) {
        name[0] = '\0';
        padding[0]= '\0';
    }
};

// Function to tokenize the path into components
std::vector<std::string> tokenize_path(const std::string& path);

// Function to read directory entries from a directory inode
int read_directory(Inode* dir_inode, std::vector<DirEntry>& entries);

// Function to resolve a file path to its inode number
int namei(const std::string& path,int& inode_num);
// Function to get the block_number
#endif 
