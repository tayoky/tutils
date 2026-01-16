#include <stdio.h>
#include <libgen.h>
#include <tutils.h>

CMD_NOPT(dirname, "dirname NAME...\n"
"strip last element from a file path\n");

static int dirname_main(int argc, char **argv){
	if(argc <= 0){
		error("missing argument");
		return 1;
	}

	for(int i=0; i<argc; i++){
		printf("%s\n", dirname(argv[i]));
	}
	return 0;
}
