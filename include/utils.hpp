#ifndef UTILS
#define UTILS

#include <algorithm>
#include <iterator>

#define BLOCK_SIZE 4096
#define INODE_BLOCKS 15

//template function to compare arrays
template <typename T, size_t N>
bool array_equal(const T (&a)[N], const T (&b)[N]) {
    return std::equal(std::begin(a), std::end(a), std::begin(b));
}

#endif