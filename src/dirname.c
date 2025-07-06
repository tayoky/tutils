#include <stdio.h>
#include <libgen.h>
#include "stdopt.h"

const char *usage = "dirname NAME...\n"
"strip last element from a file path\n";

int main(int argc,char **argv){
	parse_arg(argc,argv,NULL,0);

	if(argc < 2){
		error("missing argument");
		return 1;
	}

	for(int i=1; i<argc; i++){
		printf("%s\n",dirname(argv[i]));
	}
	return 0;
}
