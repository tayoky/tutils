#include "stdopt.h"
#include <stdio.h>
#include <unistd.h>

const char *usage = "unlink FILE\nor unlink OPTION\n"
"unlink a symlink/hardlink or remove a file\n";


int main(int argc,char **argv){
	parse_arg(argc,argv,NULL,0);
	if(argc < 2){
		error("missing argument");
		return 1;
	}
	if(argc > 2){
		error("too much arguments");
		return 1;
	}	

	if(unlink(argv[1])){
		perror(argv[1]);
		return 1;
	}
	return 0;
}
