#include <algorithm>
#include "layer2/read.h"
#include "layer2/translate_pos.h"
#include "layer0/disk_interface.hpp"
#include "layer1/data_structs.hpp"

// offset 405, size 80234
// offset 405, size 4k - 405
// offset 4k, size 4k
int read(Inode* inode, char* buf, size_t size, off_t offset) {
    if (inode == nullptr) {
        return -ENOENT;
    }
    if (size == 0 || inode->m_data.file_size <= offset) {
        return 0;
    }
    size = std::min(size_t(inode->m_data.file_size - offset), size);

    Position* pos = new Position;
    while (size > 0) {
        pos->block_num = 0;
        pos->offset = 0;
        int status;
        if ((status = translate_pos(inode, pos, offset)) != 0) {
            return -EINVAL;
        }
        size_t cur_size = std::min(size_t(BLOCK_SIZE - pos->offset), size); 
        if ((status = read_from_block(buf, pos->block_num, pos->offset, cur_size)) != cur_size) {
            return -EIO;
        }
        buf += cur_size;
        offset += cur_size;
        size -= cur_size;
    }
    return 0;
}