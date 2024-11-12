#include <stddef.h>

int read(void* buf, int block_num, size_t offset, size_t size);
int write(const void* buf, int block_num, size_t offset, size_t size);