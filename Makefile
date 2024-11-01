CC = gcc
CFLAGS = -g -Wall -D_FILE_OFFSET_BITS=64
FUSE_LIB = fuse3
TARGET = 270FileSystemProject
TEST_TARGET = 270FileSystemTests
SOURCES = main.c src/fuse_impl.c
TEST_SOURCES = tests/test_fuse_impl.cpp
TEST_MOUNT_PATH = /tmp/mountdir
OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ -l$(FUSE_LIB)

$(TEST_TARGET): $(TEST_OBJECTS) $(OBJECTS)
	$(CC) -o $@ $^ -l$(FUSE_LIB) -lgtest -lgtest_main -pthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET) $(TEST_MOUNT_PATH)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_OBJECTS) $(TEST_TARGET)

.PHONY: all clean test
