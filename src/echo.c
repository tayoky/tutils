#include <stdio.h>
#include <tutils.h>

static opt_t opts[] = {
};

CMD(echo, "echo [STRING] ...\n"
"print strings to stdout followed by a newline\n",
opts);

static int echo_main(int argc,char **argv){
	for(int i=0; i<argc; i++){
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
