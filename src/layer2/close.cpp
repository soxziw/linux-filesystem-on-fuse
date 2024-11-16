#include "layer2/close.h"
#include "layer2/file_mode.h"


int close(Inode* inode) {
    if (inode == nullptr) {
        return -ENOENT;
    }
    if (inode->m_data.file_type == FILE_TYPE::DIR) {
        return -EISDIR;
    }
    return 0;
}