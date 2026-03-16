#include <stdio.h>
#include <stdlib.h>
#include <tutils.h>

static opt_t opts[] = {
	OPT('E', "--canonicalize", 0, "all components except the last must exist"),
	OPT('e', "--canonicalize-existing", 0, "all components must exist"),
	OPT('m', "--canonicalize-missing", 0, "no components need to exist"),
};

CMD(realpath, "realpath [OPTIONS...] FILE...\n"
"resolve absolute path to a file\n",
opts);

static void rpath(char *path){
	path = realpath(path, NULL);
	printf("%s\n", path);
	free(path);
}

static int realpath_main(int argc,char **argv){
	if(argc < 1){
		error("missing argument");
		return 1;
	}

	for (int i=0; i<argc; i++) {
		rpath(argv[i]);
	}

	return 0;
}
