#include "stdopt.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

const char *usage = "unlink FILE\nor unlink OPTION\n"
"unlink a symlink/hardlink or remove a file\n";


int main(int argc,char **argv){
	parse_arg(argc,argv,NULL,0);
	if(argc < 2){
		iprintf("unlink : missing argument\n");
		return 1;
	}
	if(argc > 2){
		iprintf("unlink : too much arguments\n");
		return 1;
	}	

	if(unlink(argv[1])){
		iprintf("%s :%s\n",argv[1],strerror(errno));
		return 1;
	}
	return 0;
}
