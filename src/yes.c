#include <stdio.h>
#include "stdopt.h"

VERSION("1.0.0")

void help(){
	printf("yes [STRING]\nor yes OPTION\n");
	printf("repeatedly print a string to stdout\n");
}

int main(int argc,char **argv){
	if(argc > 2){
		iprintf("too much argument\n");
		return 1;
	}
	ARGSTART
	ARGEND

	const char *str = "y";
	if(argc == 2){
		str = argv[1];
	}

	for(;;){
		puts(str);
	}
}
