cmake_minimum_required(VERSION 3.14)
project(270FileSystemProject C CXX)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CMAKE_BUILD_TYPE Debug)


add_compile_definitions(_FILE_OFFSET_BITS=64)


include_directories(include /usr/include/fuse3)


set(SOURCES main.c src/fuse_impl.c)
add_executable(270FileSystemProject ${SOURCES})
target_link_libraries(270FileSystemProject fuse3)


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/1204d634444b0ba6da53201a8b6caf2a502d883c.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

set(TEST_SOURCES tests/main.cpp tests/test_fuse_impl.cpp src/fuse_impl.c)
add_executable(270FileSystemTests ${TEST_SOURCES})
target_link_libraries(270FileSystemTests gtest_main fuse3)

include(GoogleTest)
gtest_discover_tests(270FileSystemTests)
