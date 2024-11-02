cmake_minimum_required(VERSION 3.14)
project(270FileSystemProject CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CMAKE_BUILD_TYPE Debug)


add_compile_definitions(_FILE_OFFSET_BITS=64)


include_directories(include /usr/include/fuse3)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(270FileSystemProject ${SOURCES} main.cpp)
target_link_libraries(270FileSystemProject fuse3)


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/1204d634444b0ba6da53201a8b6caf2a502d883c.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
add_executable(270FileSystemTests ${TEST_SOURCES} ${SOURCES})
target_link_libraries(270FileSystemTests gtest_main fuse3)

file(GLOB_RECURSE INTEGRATION_SOURCES "integration/*.cpp")
add_executable(270FileSystemIntegrations ${INTEGRATION_SOURCES} ${SOURCES})
target_link_libraries(270FileSystemIntegrations gtest_main fuse3)

include(GoogleTest)
gtest_discover_tests(270FileSystemTests)
gtest_discover_tests(270FileSystemIntegrations)

# Custom build target for creating only the main executable
add_custom_target(build
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target 270FileSystemProject
    DEPENDS 270FileSystemProject
)

# Custom release target to build and run tests
add_custom_target(release
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target 270FileSystemProject
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target 270FileSystemTests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target 270FileSystemIntegrations
    COMMAND 270FileSystemTests # Run the tests
    COMMAND 270FileSystemIntegrations # Run the integration tests
    DEPENDS 270FileSystemProject 270FileSystemTests 270FileSystemIntegrations
)

# Custom clean target to remove all build files
add_custom_target(cleanup
    COMMAND ${CMAKE_COMMAND} rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build files..."
)
