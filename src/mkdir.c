#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "stdopt.h"

#define DMODE S_IRWXU | S_IRWXG 

int parent = 0;
int ret = 0;
mode_t mode = DMODE;

VERSION("indev 0.0.1")

void help(){
	iprintf("mkdir -[p] DIRECTORIES\n");
}

void make_dir(const char *path){
	//simple check to avoid segfault
	if(!path[0]){
		return;
	}
	//if parent activated cut the path
	char *parents[256];
	int parents_count = 0;
	if(parent)
	for(int i=1; path[i]; i++){
		if(path[i] == '/'){
			//check if the next one is not a "/" or a "\0"
			if(path[i+1] && path[i+1] != '/'){
				//save that
				//no strndup on tlibc whe wrinting this
				//we need that
				parents[parents_count] = strndup(path,i);
				parents_count++;
			}
		}
	}

	//make the parents first
	for(int i=0; i<parents_count; i++){
		if(mkdir(parents[i],mode)){
			//ignore aready exist error when parent mode
			if(errno == EEXIST && parent){
				continue;
			}
			iprintf("%s : %s\n",parents[i],strerror(errno));
			ret = -1;
			return;
		}
	}

	if(mkdir(path,mode)){
		if(errno == EEXIST && parent){
			return;
		}
		iprintf("%s : %s\n",path,strerror(errno));
		ret = -1;
		return;
	}
}

int main(int argc,char **argv){
	ARGSTART
	case 'p':
		parent = 1;
		break;
	case 'm':
		break;
	ARGEND

	for(int i=1; i<argc; i++){
		if(argv[i][0] == '-')continue;
		make_dir(argv[i]);
	}
	
	return 0;
}
