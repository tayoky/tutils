#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "stdopt.h"

VERSION("0.1.9")

int ret = 0;

void help(){
	iprintf("cat [FILES] ...\n");
	iprintf("show content of file or stdin\n");
	iprintf("special file name \"-\" is equivalent to stdin\n");
	iprintf("if no files is specified stdin is used by default\n");
}

void cat(const char *path){
	FILE *file;
	//"-" is stdin
	if(!strcmp(path,"-")){
		file = stdin;
	} else {
		file = fopen(path,"r");
	}

	//handle error
	if(file == NULL){
		iprintf("%s : %s\n",path,strerror(errno));
		ret = -1;
		return;
	}


	char buffer[4096];
	size_t size;
	while((size = fread(buffer,1,sizeof(buffer),file))){
		fwrite(buffer,size,1,stdout);
	}


	//don't close stdin
	if(file != stdin){
		fclose(file);
	}
}

int main(int argc,char **argv){
	//only default options
	ARGSTART
	ARGEND
	for(int i=1; i<argc;i++){
		cat(argv[i]);
	}
	//if nothing stdin by default
	if(argc - 1 <= 0){
		cat("-");
	}
	return ret;
}
