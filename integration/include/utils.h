#define FUSE_USE_VERSION 31
#include <gtest/gtest.h>


/**
 * Use popen to test command and test for error output and status.
 * @param cmd a command line to test.
 * @param ignoreError set to ignore error output of command line, should not be turn on in turn-in version.
 * @return
 */
void testCommand(const char* cmd, bool ignoreError = false);