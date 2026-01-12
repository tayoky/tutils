#include <stdio.h>
#include "stdopt.h"

struct opt opts[] = {
};

const char *usage = "echo [STRING] ...\n"
"print strings to stdout followed by a newline\n";

int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));

	for(; i<argc; i++){
		if (printf("%s ",argv[i]) < 0) {
			perror("write");
			return 1;
		}
	}
	if (printf("\n") < 0) {
		perror("write");
		return 1;
	}
	return 0;
}
