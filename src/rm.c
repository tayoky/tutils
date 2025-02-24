#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include "stdopt.h"

VERSION("beta 0.0.1")

int ret = 0;
int recursive = 0;

void help(){
	iprintf("rm -[rf] FILES DIRECTORIES ...\n");
	iprintf("-r\ndelete directory and all content\n");
	iprintf("-f\nforce delete\n");
}

int rm(const char *path){
	//get info on it
	struct stat info;
	if(lstat(path,&info)){
		iprintf("%s : %s\n",path,strerror(errno));
		ret = -1;
		return -1;
	}
	if(S_ISDIR(info.st_mode)){
		if(!recursive){
			iprintf("%s : %s\n",path,strerror(EISDIR));
			ret = -1;
			return -1;
		}

		//TODO : delete childreen here
		DIR *dir = opendir(path);
		if(dir == NULL){
			//weird error
			//permission issue ?
			iprintf("%s : %s\n",path,strerror(errno));
			ret = -1;
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
			iprintf("%s : %s\n",path,strerror(errno));
			ret = -1;
			return -1;
		}
		return 0;
	}

	if(unlink(path)){
		iprintf("%s : %s\n",path,strerror(errno));
		ret = -1;
		return -1;
	}
	return 0;
}

int main(int argc,char **argv){
	ARGSTART
	case 'r':
		recursive = 1;
		break;
	case 'f':
		break;
	ARGEND
	
	for(int i=1; i<argc; i++){
		if(argv[i][0] == '-')continue;
		rm(argv[i]);
	}
	
	return ret ;
}
