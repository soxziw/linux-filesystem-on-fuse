#include "layer2/mkdir.h"
#include "layer2/translate_pos.h"
#include "layer0/disk_interface.hpp"
int my_mkdir(const std::string& path, mode_t mode) {
    // Validate the path
    if (path.empty() || path[0] != '/') {
        return -EINVAL; // Invalid path
    }

    // Parse the path into parent path and directory name
    size_t pos = path.find_last_of('/');
    std::string parent_path, dir_name;
    if (pos == 0) {
        // Path is like "/dirname"
        parent_path = "/";
        dir_name = path.substr(1);
    } else {
        parent_path = path.substr(0, pos);
        dir_name = path.substr(pos + 1);
    }

    if (dir_name.empty() || dir_name.length() > NAME_MAX) {
        return -EINVAL; // Directory name cannot be empty or too long
    }

    // Resolve parent directory inode number
    int parent_inode_num;
    int res = namei(parent_path, parent_inode_num);
    if (res != 0) {
        return res; // Parent directory not found
    }

    // Read parent inode
    Inode parent_inode;
    res = read_inode(parent_inode_num, parent_inode);
    if (res != 0) {
        return res; // Error reading parent inode
    }

    // Ensure parent is a directory
    if (parent_inode.m_data.file_type != 1) { // 1 for directory
        return -ENOTDIR;
    }

    // Check if directory already exists
    int existing_inode_num;
    res = namei(path, existing_inode_num);
    if (res == 0) {
        return -EEXIST; // Directory already exists
    }

    // Allocate new inode for the directory
    int new_inode_num = alloc_inode(1); // 1 for directory
    if (new_inode_num == -1) {
        return -ENOSPC; // No space left on device
    }

    InodeMData mdata;
    mdata.inode_num = new_inode_num;
    mdata.mode = mode;
    mdata.file_type = 1; // Directory
    mdata.num_hard_links = 2; // '.' and 'dir itself'
    // // maybe need fuse_context to get uid and gid
    // // new_inode.m_data.owner_id = getuid(); 
    // // new_inode.m_data.group_id = getgid();
    mdata.file_size = 2 * sizeof(DirEntry); // '.' and '..'
    mdata.creation_time = time(NULL);
    mdata.mtime = mdata.creation_time;
    mdata.atime = mdata.creation_time;
    mdata.ctime = mdata.creation_time;
    mdata.num_allocated_blocks = 1;
    // Initialize the new directory inode
    //Inode new_inode;
    res=write_inode_mdata(mdata,new_inode_num);
    if (res != 0) {
        return -1; // write fail
    }

    // Allocate a data block for the new directory
    int block_num = alloc_block();
    if (block_num == -1) {
        free_inode(new_inode_num);
        return -ENOSPC; // No space left on device
    }


    // Create '.' and '..' entries
    DirEntry dot_entry;
    dot_entry.inode_num = new_inode_num;
    strncpy(dot_entry.name, ".", NAME_MAX);

    DirEntry dotdot_entry;
    dotdot_entry.inode_num = parent_inode_num;
    strncpy(dotdot_entry.name, "..", NAME_MAX);


    // for .. & . write into first block first and second place
    //memcpy(block.dir_block,&dotdot_entry,DIRENTRY_SIZE);
    //memcpy(block.dir_block+DIRENTRY_SIZE,&dot_entry,DIRENTRY_SIZE);
    write_to_block((char*)(&dotdot_entry),block_num,0,DIRENTRY_SIZE);
    write_to_block((char*)(&dot_entry),block_num,DIRENTRY_SIZE,DIRENTRY_SIZE);
    //should invoke write_inode function in layer 1 interface 
    write_inode(new_inode_num,block_num,0);

    
    //

    // TODO:
    // Add the new directory entry to the parent directory
    //get idx of block still remain spaces in parent_inode
    Position position;
    translate_pos(&parent_inode,&position,parent_inode.m_data.file_size,true);
    //write_inode(parent_inode_num,position.block_num,idx);
    DirEntry new_file;
    new_file.inode_num=new_inode_num;
    strncpy(new_file.name,dir_name.c_str(),NAME_MAX);
    write_to_block((char*)(&new_file),position.block_num,position.offset,DIRENTRY_SIZE);
    return 0; // Success
}