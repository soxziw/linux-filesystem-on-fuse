cmake_minimum_required(VERSION 3.14)
project(270FileSystemProject CXX)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(CMAKE_BUILD_TYPE Debug)


add_compile_definitions(_FILE_OFFSET_BITS=64)


include_directories(include /usr/include/fuse3)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(fuse ${SOURCES} main.cpp)
target_link_libraries(fuse fuse3)


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/1204d634444b0ba6da53201a8b6caf2a502d883c.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
add_executable(fuseTests ${TEST_SOURCES} ${SOURCES})
target_link_libraries(fuseTests gtest_main fuse3)

file(GLOB_RECURSE INTEGRATION_SOURCES "integration/*.cpp")
add_executable(fuseIntegrations ${INTEGRATION_SOURCES} ${SOURCES})
target_include_directories(fuseIntegrations PRIVATE integration/include include /usr/include/fuse3)
target_link_libraries(fuseIntegrations gtest_main fuse3)

include(GoogleTest)
gtest_discover_tests(fuseTests)
gtest_discover_tests(fuseIntegrations)

# Custom build target for creating only the main executable
add_custom_target(build
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fuse
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fuseTests
    COMMAND fuseTests # Run the unit tests
    DEPENDS fuse fuseTests
)

# Custom release target to build and run tests
add_custom_target(release
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fuse
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fuseTests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fuseIntegrations
    COMMAND fuseTests # Run the unit tests
    COMMAND fuseIntegrations # Run the integration tests
    DEPENDS fuse fuseTests fuseIntegrations
)

# Custom clean target to remove all build files
add_custom_target(cleanup
    COMMAND ${CMAKE_COMMAND} rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build files..."
)
