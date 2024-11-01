#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


// void test_read_existing_file(const char *path, const char *expected_content) {
//     printf("Test 1: Read Existing File\n");
//     FILE *fp = fopen(path, "r");
//     if (fp == NULL) {
//         perror("Failed to open file");
//         return;
//     }
//     char buf[1024];
//     memset(buf, 0, sizeof(buf));
//     if (fgets(buf, sizeof(buf), fp) != NULL) {
//         if (strcmp(buf, expected_content) == 0) {
//             printf("Success: Content matches expected content.\n");
//         } else {
//             printf("Failure: Content does not match expected content.\n");
//         }
//     } else {
//         if (feof(fp)) {
//             printf("Failure: End of file reached unexpectedly.\n");
//         } else {
//             perror("Failed to read from file");
//         }
//     }
//     fclose(fp);
// }

// void test_write_existing_file(const char *path, const char *content) {
//     printf("Test 2: Write to Existing File\n");
//     FILE *fp = fopen(path, "w");
//     if (fp == NULL) {
//         perror("Failed to open file for writing");
//         return;
//     }
//     if (fputs(content, fp) == EOF) {
//         perror("Failed to write to file");
//     } else {
//         printf("Success: Content written successfully.\n");
//     }
//     fclose(fp);
// }

void test_write_new_file(const char *path, const char *content) {
printf("\nTest 3: Write to Non-Existent File using system calls\n");

    int fd = open(path, O_WRONLY | O_CREAT, 0666);
    if (fd == -1) {
        perror("Failed to create and open file for writing");
        return;
    }

    ssize_t bytes_written = write(fd, content, strlen(content));
    if (bytes_written == -1) {
        perror("Failed to write to file");
        close(fd);
        return;
    }

    if ((size_t)bytes_written != strlen(content)) {
        printf("Failure: Not all bytes were written to the file.\n");
    } else {
        printf("Success: Content written successfully to new file.\n");
    }

    close(fd);
}

int main() {
    const char *mount_point = "/cs270";
    const char *existing_file = "/cs270/filename";
    const char *nonexistent_file = "/cs270/newfile";
    const char *expected_content = "Hello, World!\n";
    const char *write_content = "Sample content\n";

    // test_read_existing_file(existing_file, expected_content);
    // test_write_existing_file(existing_file, write_content);
    test_write_new_file(nonexistent_file, write_content);

    return 0;
}