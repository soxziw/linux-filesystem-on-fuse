#include "layer2/open.h"
#include "layer2/utils.h"


int open(Inode* inode, int flags, fuse_context* context) {
    if (inode == nullptr) {
        return -ENOENT;
    }
    if (context->uid == 0) { // root
        return 0;
    }
    if (inode->m_data.owner_id == context->uid
         && (inode->m_data.mode & OWNER_MODE) == flags) { // same owner with permission
        return 0;
    } else if (inode->m_data.group_id == context->gid
         && (inode->m_data.mode & GROUP_MODE) == flags) { // same group with permission
        return 0;
    } else if ((inode->m_data.mode & OTHER_MODE) == flags) { // other with permission
        return 0;
    }

    return -EACCES;
}