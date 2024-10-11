fuse: fuse.cpp
	g++ -o fuse fuse.cpp -lfuse -D_FILE_OFFSET_BITS=64