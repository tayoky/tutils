#include <dirent.h>
#include <stdio.h>
#include <tutils.h>

struct opt opts[] = {
};

CMD(find, "find [PATH...] [EXPR...]\n"
"find files\n", opts);

static int find_main(int argc, char **argv) {
	error("TODO : impl find");
	return 1;
}
