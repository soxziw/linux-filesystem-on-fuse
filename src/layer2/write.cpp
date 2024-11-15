#include <algorithm>
#include "layer2/write.h"
#include "layer2/translate_pos.h"
#include "mock_layer0.h"
// file_size 405
// offset 18321
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
        if ((status = write_to_block(buf, pos->block_num, pos->offset, cur_size)) != cur_size) {
            return -errno;
        }
        buf += cur_size;
        offset += cur_size;
        size -= cur_size;
    }

    inode->m_data.file_size = std::max(long(offset + size), inode->m_data.file_size);
    write_inode_mdata(inode->m_data, inode->m_data.inode_num);
    return 0;
}