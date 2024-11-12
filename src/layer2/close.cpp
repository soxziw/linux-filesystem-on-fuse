#include "layer2/close.h"
#include "layer2/utils.h"


int close(Inode* inode) {
    if (inode == nullptr) {
        return -ENOENT;
    }
    return 0;
}