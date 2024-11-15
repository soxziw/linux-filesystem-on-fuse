// #include <gtest/gtest.h>
// #include <gmock/gmock.h>
// #include <string>
// #include <cstring>
// #include <cerrno>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <cstdlib>
// #include "layer2/read.h"
// #include "layer2/translate_pos.h"

// class MockClass {
// public:
//     MOCK_METHOD4(translate_pos, int(Inode*, Position*, int, bool));
//     MOCK_METHOD4(read_from_block, int(char*, size_t, unsigned short, size_t));
// };

// TEST(FuseTest, Layer2_Read_ValidRead) {
//     MockClass mock;
//     Inode* file_inode = new Inode;
//     Position* pos = new Position;
//     EXPECT_CALL(mock, translate_pos(file_inode, pos, 10, true))
//         .WillOnce(::testing::DoAll(
//             ::testing::SetArgPointee<1>(new Position{100, 10}),
//             ::testing::Return(0)
//         ));

//     char* base_content;
//     EXPECT_CALL(mock, read_from_block(base_content, 100, 10, 7))
//         .WillOnce(::testing::DoAll(
//             ::testing::SetArgPointee<0>("hello\n"),
//             ::testing::Return(0)
//         ));
    
//     char* content;
//     int status;
//     status = read(file_inode, content, 7, 10);
//     ASSERT_EQ(status, 0);
//     ASSERT_STREQ(content, base_content);
// }

// TEST(FuseTest, Layer2_Read_ValidReadInvalidPosition) {
//     MockClass mock;
//     Inode* file_inode = new Inode;
//     Position* pos = new Position;
//     EXPECT_CALL(mock, translate_pos(file_inode, pos, 10, true))
//         .WillOnce(::testing::DoAll(
//             ::testing::SetArgPointee<1>(new Position{0, 0}),
//             ::testing::Return(-EINVAL)
//         ));

//     char* base_content;
//     EXPECT_CALL(mock, read_from_block(base_content, 0, 0, 7))
//         .WillOnce(::testing::DoAll(
//             ::testing::SetArgPointee<0>("hello\n"),
//             ::testing::Return(0)
//         ));
    
//     char* content;
//     int status;
//     status = read(file_inode, content, 7, 10);
//     ASSERT_EQ(status, -EINVAL);
//     ASSERT_STREQ(content, "");
// }

// TEST(FuseTest, Layer2_Read_ValidReadInvalidBlock) {
//     MockClass mock;
//     Inode* file_inode = new Inode;
//     Position* pos = new Position;
//     EXPECT_CALL(mock, translate_pos(file_inode, pos, 10, true))
//         .WillOnce(::testing::DoAll(
//             ::testing::SetArgPointee<1>(new Position{100, 10}),
//             ::testing::Return(0)
//         ));

//     char* base_content;
//     EXPECT_CALL(mock, read_from_block(base_content, 100, 10, 7))
//         .WillOnce(::testing::DoAll(
//             ::testing::SetArgPointee<0>(""),
//             ::testing::Return(-EIO)
//         ));
    
//     char* content;
//     int status;
//     status = read(file_inode, content, 7, 10);
//     ASSERT_EQ(status, -EIO);
//     ASSERT_STREQ(content, "");
// }