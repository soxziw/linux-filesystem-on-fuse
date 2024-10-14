fuse: fuse.c
	gcc -Wall fuse.c `pkg-config fuse3 --cflags --libs` -o fuse