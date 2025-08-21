#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "stdopt.h"

#define FLAG_PARENT 0x08
int ret = 0;
mode_t mode = S_IRWXU;
char *m = NULL;

struct opt opts[] = {
	OPT('p',"--parent",FLAG_PARENT,"make parent directories if needed and ignore if any directory aready exist"),
	OPTV('m',"--mode",0,&m,"change mode of the news directories"),
};


const char *usage = "mkdir [-m MODE] [-p] DIRECTORIES ...\n"
"create direcories\n";

void make_dir(const char *path){
	//simple check to avoid segfault
	if(!path[0]){
		return;
	}
	//if parent activated cut the path
	char *parents[256];
	int parents_count = 0;
	if(flags & FLAG_PARENT)
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
			if(errno == EEXIST && (flags & FLAG_PARENT)){
				continue;
			}
			perror(parents[i]);
			ret = 1;
			return;
		}
	}

	if(mkdir(path,mode)){
		if(errno == EEXIST && (flags & FLAG_PARENT)){
			return;
		}
		perror(path);
		ret = 1;
		return;
	}
}

int main(int argc,char **argv){
	parse_arg(argc,argv,opts,arraylen(opts));
	if(m){
		//TODO : parse m
	}

	int dir_count = 0;

	for(int i=1; i<argc; i++){
		if(argv[i][0] == '-')continue;
		make_dir(argv[i]);
		dir_count ++;
	}

	if(!dir_count){
		error("missing argument");
		return 1;
	}
	
	return ret;
}
