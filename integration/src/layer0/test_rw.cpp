#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include "layer0/disk_interface.hpp"


TEST(FuseIntegration, Layer0_WriteReadToSameArea) {
    memset(in_mem_disk, 0, sizeof(in_mem_disk));
    int status;

    const char* content = "test component\n";
    status = write_to_block(content, 0, 0, strlen(content));
    ASSERT_EQ(status, strlen(content));
    ASSERT_EQ('p', in_mem_disk[8]);

    char* new_content = new char[strlen(content)];
    status = read_from_block(new_content, 0, 0, strlen(content));
    ASSERT_EQ(status, strlen(content));
    ASSERT_STREQ(content, new_content);
}

TEST(FuseIntegration, Layer0_WriteReadToDiffArea) {
    memset(in_mem_disk, 0, sizeof(in_mem_disk));
    int status;

    const char* content = "test component\n";
    status = write_to_block(content, 0, 0, strlen(content));
    ASSERT_EQ(status, strlen(content));

    char* new_content = new char[strlen(content)];
    status = read_from_block(new_content, 1, 0, strlen(content));
    ASSERT_EQ(status, strlen(content));
    ASSERT_STRNE(content, new_content);
}

TEST(FuseIntegration, Layer0_WriteReadToEndOfBlock) {
    memset(in_mem_disk, 0, sizeof(in_mem_disk));
    int status;

    const char* content = "test component\n";
    status = write_to_block(content, 0, 4096 - 1 - strlen(content), strlen(content));
    ASSERT_EQ(status, strlen(content));

    char* new_content = new char[strlen(content)];
    status = read_from_block(new_content, 0, 4096 - 1 - strlen(content), strlen(content));
    ASSERT_EQ(status, strlen(content));
    ASSERT_STREQ(content, new_content);
}