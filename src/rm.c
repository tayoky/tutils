#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include "stdopt.h"

#define FLAG_RECURSIVE 0x08
#define FLAG_FORCE     0x10

struct opt opts[] = {
	OPT('f',"--force",FLAG_FORCE,"force delete"),
	OPT('r',"--recursive",FLAG_RECURSIVE,"delete directories and their content"),
};

int ret = 0;

const char *usage = "rm [-rf] FILES DIRECTORIES ...\n"
"delete files\n";

int rm(const char *path){
	//get info on it
	struct stat info;
	if(lstat(path,&info)){
		//when force
		//not existing is not a problem
		if((flags & FLAG_FORCE) && errno == ENOENT){
			return 0;
		}
		perror(path);
		ret = 1;
		return -1;
	}
	if(S_ISDIR(info.st_mode)){
		if(!(flags & FLAG_RECURSIVE)){
			errno = EISDIR;
			perror(path);
			ret = 1;
			return -1;
		}

		DIR *dir = opendir(path);
		if(dir == NULL){
			//weird error
			//permission issue ?
			perror(path);
			ret = 1;
			return -1;
		}

		for(;;){
			struct dirent *ret = readdir(dir);
			if(!ret)break;

			//ignore . and ..
			if(!strcmp(ret->d_name,".")){
					continue;
			}
			if(!strcmp(ret->d_name,"..")){
				continue;
			}

			//find the full name
			char *full_name = malloc(strlen(ret->d_name) + strlen(path) + 2);
			sprintf(full_name,"%s/%s",path,ret->d_name);

			//that why it's recursive
			rm(full_name);
		}

		//delete the directory
		if(rmdir(path)){
			perror(path);
			ret = 1;
			return -1;
		}
		return 0;
	}

	if(unlink(path)){
		perror(path);
		ret = 1;
		return -1;
	}
	return 0;
}

int main(int argc,char **argv){
	parse_arg(argc,argv,opts,arraylen(opts));
	
	int count = 0;
	for(int i=1; i<argc; i++){
		if(argv[i][0] == '-')continue;
		count ++;
		rm(argv[i]);
	}
	if(!count){
		error("missing argument");
		return 1;
	}
	
	return ret ;
}
