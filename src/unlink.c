#include "stdopt.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

VERSION("0.1.0")

void help(){
	iprintf("unlink FILE\nor unlink OPTION\n");
	iprintf("unlink a symlink/hardlink or remove a file\n");
}


int main(int argc,char **argv){
	if(argc < 2){
		iprintf("unlink : missing argument\n");
		return 1;
	}
	if(argc > 2){
		iprintf("unlink : too much arguments\n");
		return 1;
	}
	
	//only default arguments
	ARGSTART
	ARGEND

	if(unlink(argv[1])){
		iprintf("%s :%s\n",argv[1],strerror(errno));
		return 1;
	}
	return 0;
}
