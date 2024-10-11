#include <fuse.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

// Forward declarations of the filesystem functions
static int fuse_read(const char *path, char *buf, size_t size, off_t offset);

static struct fuse_operations fuse_example_operations = {
    .read = &fuse_read
};

static int fuse_read(const char *path, char *buf, size_t size, off_t offset) {
    printf("hello world\n");
    return 0;
}

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &fuse_example_operations);
}