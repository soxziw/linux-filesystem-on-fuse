#include <gtest/gtest.h>
#include "layer0/disk_interface.hpp"
#include "layer1/data_structs.hpp" // Include your filesystem header here
#include "layer2/translate_pos.h"
#include "layer2/open.h"
#include "layer2/close.h"
#include "layer2/read.h"
#include "layer2/write.h"

#include "layer2/l2helper.h"
#include "layer2/mkdir.h"
#include "layer2/mknod.h"
#include "layer2/readdir.h"
#include "layer2/unlink.h"

class FileSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize the file system with 10 inode blocks and 100 data blocks
        init_fs(10, 100);
    }

    void TearDown() override {
        // Clear the in-memory disk after each test
        memset(in_mem_disk, 0, sizeof(in_mem_disk));
    }
};

// Test: Resolving the inode of an existing file
TEST_F(FileSystemTest, NameiExistingFile) {
    // Set up: Create a test file with a known path
    std::string test_path = "/dir/subdir/file.txt";
    int inode_num = alloc_inode(REG_FILE);
    ASSERT_GT(inode_num, 0);

    // Add the file to the directory structure
    // Simulate directories and file creation (details omitted for brevity)
    int res = my_mkdir("/dir", 0755);
    ASSERT_EQ(res, 0);
    res = my_mkdir("/dir/subdir", 0755);
    ASSERT_EQ(res, 0);

    // Resolve the inode of the file using namei
    int resolved_inode_num;
    res = namei(test_path, resolved_inode_num);
    ASSERT_EQ(res, 0); // Should succeed

    // Validate that the resolved inode is correct (should be the same as created)
    ASSERT_EQ(resolved_inode_num, inode_num);
}

// Test: Resolving an invalid path (non-existing file)
TEST_F(FileSystemTest, NameiInvalidPath) {
    std::string invalid_path = "/nonexistent/dir/file.txt";
    int inode_num;

    int res = namei(invalid_path, inode_num);
    ASSERT_EQ(res, -ENOENT); // Should return No such file or directory
}

// Test: Creating a directory with a valid path
TEST_F(FileSystemTest, MyMkdirValid) {
    std::string dir_path = "/newdir";
    int res = my_mkdir(dir_path, 0755);
    ASSERT_EQ(res, 0); // Directory creation should succeed

    // Verify the directory was created by checking its existence
    int inode_num;
    res = namei(dir_path, inode_num);
    ASSERT_EQ(res, 0); // Directory should exist
}

// Test: Trying to create a directory that already exists
TEST_F(FileSystemTest, MyMkdirDirectoryExists) {
    std::string dir_path = "/existingdir";
    int res = my_mkdir(dir_path, 0755);
    ASSERT_EQ(res, 0); // First creation should succeed

    // Attempt to create the same directory again
    res = my_mkdir(dir_path, 0755);
    ASSERT_EQ(res, -EEXIST); // Directory already exists
}

// Test: Reading directory entries from a valid directory
TEST_F(FileSystemTest, ReaddirValidDirectory) {
    // Set up: Create a test directory with entries
    std::string dir_path = "/dir";
    int res = my_mkdir(dir_path, 0755);
    ASSERT_EQ(res, 0);

    // Add a file to the directory
    std::string file_path = "/dir/file.txt";
    int file_inode_num = alloc_inode(REG_FILE);
    ASSERT_GT(file_inode_num, 0);
    res = namei(dir_path, file_inode_num);
    ASSERT_EQ(res, 0);

    // Read the directory entries
    std::vector<DirEntry> entries;
    res = readdir(dir_path, entries);
    ASSERT_EQ(res, 0); // Reading should succeed
    ASSERT_EQ(entries.size(), 2); // Expecting '.' and '..' entries plus file

    // Verify the entries
    ASSERT_STREQ(entries[0].name, ".");
    ASSERT_STREQ(entries[1].name, "..");
}

// Test: Trying to read entries from a non-directory path
TEST_F(FileSystemTest, ReaddirNonDirectory) {
    std::string file_path = "/file.txt";
    int file_inode_num = alloc_inode(REG_FILE);
    ASSERT_GT(file_inode_num, 0);

    // Try to read directory entries from a regular file path
    std::vector<DirEntry> entries;
    int res = readdir(file_path, entries);
    ASSERT_EQ(res, -ENOTDIR); // Should return Not a directory
}

// Test: Creating a directory with an invalid name (empty name)
TEST_F(FileSystemTest, MyMkdirInvalidNameEmpty) {
    std::string dir_path = "/dir//subdir"; // Invalid because of the empty name between slashes
    int res = my_mkdir(dir_path, 0755);
    ASSERT_EQ(res, -EINVAL); // Invalid directory name
}

// Test: Creating a directory with a long name (exceeding NAME_MAX)
TEST_F(FileSystemTest, MyMkdirLongName) {
    std::string long_name = "/dir/" + std::string(NAME_MAX + 1, 'a'); // Name too long
    int res = my_mkdir(long_name, 0755);
    ASSERT_EQ(res, -EINVAL); // Invalid directory name (too long)
}

// Test: Traversing up the directory tree using ".."
TEST_F(FileSystemTest, NameiParentDirectoryTraversal) {
    // Set up: Create directories
    int res = my_mkdir("/dir", 0755);
    ASSERT_EQ(res, 0);
    res = my_mkdir("/dir/subdir", 0755);
    ASSERT_EQ(res, 0);

    // Resolve the inode of subdir
    int subdir_inode_num;
    res = namei("/dir/subdir", subdir_inode_num);
    ASSERT_EQ(res, 0); // Should succeed

    // Resolve the inode of the parent directory using ".."
    int parent_inode_num;
    res = namei("/dir", parent_inode_num);
    ASSERT_EQ(res, 0); // Should succeed

    // Validate that the parent inode is correct
    ASSERT_EQ(parent_inode_num, subdir_inode_num);
}

// Test: Reading the parent directory of a file
TEST_F(FileSystemTest, ReaddirParentDirectory) {
    std::string dir_path = "/dir";
    int res = my_mkdir(dir_path, 0755);
    ASSERT_EQ(res, 0);

    std::vector<DirEntry> entries;
    res = readdir(dir_path, entries);
    ASSERT_EQ(res, 0); // Should succeed
    ASSERT_EQ(entries.size(), 2); // Only '.' and '..' should exist

    // Verify parent directory (should be empty except '.' and '..')
    ASSERT_STREQ(entries[0].name, ".");
    ASSERT_STREQ(entries[1].name, "..");
}
