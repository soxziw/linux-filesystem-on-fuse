#define FUSE_USE_VERSION 31

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <vector>
#include "fuse_impl.h"


// #define OPTION(t, p)                           \
//     { t, offsetof(struct options, p), 1 }
// static const struct fuse_opt option_spec[] = {
// 	OPTION("--name=%s", filename),
// 	OPTION("--contents=%s", contents),
// 	OPTION("-h", show_help),
// 	OPTION("--help", show_help),
// 	FUSE_OPT_END
// };


static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}


int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	for (int i = 0; i < OPEN_FILE_LIST_SIZE; i++) {
		open_files[i].filename.clear();
	}

	ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
	fuse_opt_free_args(&args);

	return ret;
}