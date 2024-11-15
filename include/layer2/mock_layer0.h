#include <stddef.h>

int read_from_block(char* dest_buf, size_t block, unsigned short offset, size_t size);
int write_to_block(const char* src_buf, size_t block, unsigned short offset, size_t size);