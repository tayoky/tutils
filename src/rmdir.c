#include <unistd.h>
#include <stdio.h>
#include <tutils.h>

#define FLAG_PARENT  0x1
#define FLAG_VERBOSE 0x2

static int ret = 0;

static opt_t opts[] = {
	OPT('p',"--parents",FLAG_PARENT,"also remove parents directories"),
	OPT('v',"--verbose",FLAG_VERBOSE,"print a message for each removed directory"),
};

CMD(rmdir, "rmdir [-pv] DIRECTORIES ...\n"
"remove directories\n",
opts);

static int remove_dir(const char *path) {
	if (rmdir(path) < 0) {
		perror(path);
		ret = 1;
		return -1;
	}
	if (flags & FLAG_VERBOSE) {
		printf("rmdir : removed directory '%s'\n", path);
	}

	return 0;
}

static void do_rmdir(char *path) {
	for (;;) {
		if (remove_dir(path) < 0) {
			break;
		}
		if (!(flags & FLAG_PARENT)) {
			break;
		}

		// first remove any trailing /
		char *end = path + strlen(path);
		while (end > path && (!*end || *end == '/')) end--;
		if (end == path) {
			// no more parent
			break;
		}
		// now remove the last element of the path
		while (end > path && *end != '/') end--;
		if (end == path) {
			// no more parent
			break;
		}

		end++;
		*end = '\0';
	}
}

static int rmdir_main(int argc, char **argv) {
	if (argc < 1) {
		error("missing argument");
		return 1;
	}

	for (int i=0; i<argc; i++) {
		do_rmdir(argv[i]);
	}

	return ret;
}
