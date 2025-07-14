#include <stdio.h>
#include "stdopt.h"


const char *usage = "yes [STRING]\nor yes OPTION\n"
"repeatedly print a string to stdout\n";

int main(int argc,char **argv){
	parse_arg(argc,argv,NULL,0);

	for(;;){
		if(argc > 1){
			for(int i=1;i<argc;i++){
				fputs(argv[i],stdout);
				putchar(i == argc-1 ? '\n' : ' ');
			}
		} else {
			puts("y");
		}
	}
}
