#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "stdopt.h"

#define FLAG_P 0x08
#define FLAG_L 0x10
struct opt opts[] = {
	OPT('P',NULL,FLAG_P,"show the $PWD variable"),
	OPT('L',NULL,FLAG_L,"show the pysical path without sysmlink"),
};

const char *usage = "pwd [-L]\n"
"or pwd -P\n"
"print the working directory\n"
"by default pwd behave like called with -L\n";



int main(int argc,char **argv){
	parse_arg(argc,argv,opts,arraylen(opts));

	if(flags & FLAG_P){
		char cwd[256];
		getcwd(cwd,256);
		printf("%s\n",cwd);
	} else {
		char *pwd = getenv("PWD");
		printf("%s\n",pwd);
	}
	return 0;
}
