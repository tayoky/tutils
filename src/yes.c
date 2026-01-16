#include <stdio.h>
#include <tutils.h>

CMD_NOPT(yes, "yes [STRING]\nor yes OPTION\n"
"repeatedly print a string to stdout\n");

static int yes_main(int argc,char **argv){
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
