#include <stdlib.h>
#include <stdio.h>

int main() {
    int fd = open("/270/test.txt", O_WRITE);
             write(fd, "test");
}