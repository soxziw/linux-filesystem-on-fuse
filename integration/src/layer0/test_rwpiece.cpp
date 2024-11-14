#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include "layer2/mock_layer0.h"

TEST(FuseIntegration, Layer0_WriteReadToSameBlock) {
    int status;

    const char* content = "test component\n";
    status = write(content, 0, 0, sizeof(content));
    ASSERT_EQ(status, sizeof(content));

    char* new_content;
    status = read(new_content, 0, 0, sizeof(content));
    ASSERT_EQ(status, sizeof(content));
    ASSERT_STREQ(content, new_content);
}

TEST(FuseIntegration, Layer0_WriteReadToDiffBlock) {
    int status;

    const char* content = "test component\n";
    status = write(content, 0, 0, sizeof(content));
    ASSERT_EQ(status, sizeof(content));

    char* new_content;
    status = read(new_content, 1, 0, sizeof(content));
    ASSERT_EQ(status, -1);
    ASSERT_STRNE(content, new_content);
}

TEST(FuseIntegration, Layer0_WriteReadToEndOfBlock) {
    int status;

    const char* content = "test component\n";
    status = write(content, 0, 4096 - sizeof(content), sizeof(content));
    ASSERT_EQ(status, sizeof(content));

    char* new_content;
    status = read(new_content, 0, 4096 - sizeof(content), sizeof(content));
    ASSERT_EQ(status, sizeof(content));
    ASSERT_STREQ(content, new_content);
}