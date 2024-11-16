#include "layer2/open.h"
#include "layer2/file_mode.h"


int open(Inode* inode, int flags, fuse_context* context) {
    if (inode == nullptr) {
        return -ENOENT;
    }
    if (inode->m_data.file_type == FILE_TYPE::DIR) {
        return -EISDIR;
    }
    if (context->uid == 0) { // root
        return 0;
    }
    if (inode->m_data.owner_id == context->uid
         && GET_OWNER_MODE(inode->m_data.mode) == flags) { // same owner with permission
        return 0;
    } else if (inode->m_data.group_id == context->gid
         && GET_GROUP_MODE(inode->m_data.mode) == flags) { // same group with permission
        return 0;
    } else if (GET_OTHER_MODE(inode->m_data.mode) == flags) { // other with permission
        return 0;
    }

    return -EACCES;
}