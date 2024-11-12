#include <algorithm>
#include "layer2/write.h"
#include "layer2/utils.h"
#include "mock_layer0.h"

int write(Inode* inode, const char* buf, size_t size, off_t offset) {
    if (inode == nullptr) {
        return -ENOENT;
    }
    if (size == 0) {
        return 0;
    }

    Position* pos = new Position;
    for(int idx = (inode->m_data.file_size + BLOCK_SIZE) / BLOCK_SIZE * BLOCK_SIZE; idx < offset; idx += BLOCK_SIZE) {
        int status;
        if ((status = translate_pos(inode, pos, offset, true)) != 0) {
            return -errno;
        }
    }

    while (size > 0) {
        pos->block_num = 0;
        pos->offset = 0;
        int status;
        if ((status = translate_pos(inode, pos, offset, true)) != 0) {
            return -errno;
        }
        size_t cur_size = std::min(size_t(BLOCK_SIZE - pos->offset), size); 
        if ((status = write((const void*)buf, pos->block_num, pos->offset, cur_size)) != cur_size) {
            return -errno;
        }
        buf += cur_size;
        offset += cur_size;
        size -= cur_size;
    }
    return 0;
}