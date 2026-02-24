#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <tutils.h>

#define FLAG_ALL 0x01

static opt_t opts[] = {
	OPT('a', "--all", FLAG_ALL ,"show all matches"),
};

CMD(which, "which [-a] EXECUTABLE...\n"
"show full path for executables in $PATH\n", opts);

static int ret = 0;

static void do_which(const char *exe) {
	struct stat st;
	// check if it is a aready a path (contiain at least one '/')
	if (strchr(exe, '/')) {
		if (stat(exe, &st) < 0) {
			ret = 1;
		} else {
			puts(exe);
		}
		return;
	}

	// this is where the actual PATH work is done
	char *path = getenv("PATH");
	if (!path) {
		// no path ?
		ret = 1;
		return;
	}

	// we are going to modify this
	char *d = strdup(path);
	path = d;

	size_t path_count = 1;

	for (size_t i = 0;path[i]; i++){
		if(path[i] == ':'){
			path_count++;
			path[i] = '\0';
		}
	}

	int found = 0;
	for (size_t i = 0; i < path_count; i++) {
		// is what we search here ?
		char *full_filename = malloc(strlen(path) + strlen(exe) + 2);
		sprintf(full_filename, "%s/%s", path, exe);
		if (stat(full_filename, &st) >= 0) {
			found = 1;
			puts(full_filename);
		}
		free(full_filename);
		if (found && !(flags & FLAG_ALL)) {
			break;
		}

		// go to the next one
		path += strlen(path) + 1;
	}
	if (!found) ret = 1;

	free(d);
}

static int which_main(int argc, char **argv) {
	if (argc < 1) {
		error("missing argument");
		return 1;
	}
	for (int i=0; i<argc; i++) {
		do_which(argv[i]);
	}
	return ret;
}
