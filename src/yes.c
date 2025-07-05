#include <stdio.h>
#include "stdopt.h"


const char *usage = "yes [STRING]\nor yes OPTION\n"
"repeatedly print a string to stdout\n";

int main(int argc,char **argv){
	if(argc > 2){
		iprintf("too much argument\n");
		return 1;
	}

	parse_arg(argc,argv,NULL,0);

	const char *str = "y";
	if(argc == 2){
		str = argv[1];
	}

	for(;;){
		puts(str);
	}
}
