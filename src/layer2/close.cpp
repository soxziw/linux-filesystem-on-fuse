#include "layer2/close.h"
#include "layer2/translate_pos.h"


int close(Inode* inode) {
    if (inode == nullptr) {
        return -ENOENT;
    }
    return 0;
}