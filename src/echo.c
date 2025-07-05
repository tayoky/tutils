#include <stdio.h>
#include "stdopt.h"

struct opt opts[] = {
};

const char *usage = "echo [STRING] ...\n"
"print strings to stdout followed by a newline\n";

int main(int argc,char **argv){
	parse_arg(argc,argv,opts,arraylen(opts));

	for(int i=1; i<argc; i++){
		printf("%s ",argv[i]);
	}
	printf("\n");
}
