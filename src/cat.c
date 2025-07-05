#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "stdopt.h"


int ret = 0;

void help(){
	printf("cat [FILES] ...\n");
	printf("concatenate files and print to stdout\n");
	printf("special file name \"-\" is equivalent to stdin\n");
	printf("if no files is specified stdin is used by default\n");
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
		ret = 1;
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

struct opt opts[] = {
	OPT('h',"--help",1),
};

int main(int argc,char **argv){
	parse_arg(argc,argv,opts,arraylen(opts));
	if(flags){
		help();
		return 0;
	}

	for(int i=1; i<argc;i++){
		cat(argv[i]);
	}
	//if nothing stdin by default
	if(argc - 1 <= 0){
		cat("-");
	}
	return ret;
}
