#include <sys/stat.h>
#include <stdio.h>
#include "stdopt.h"

//TODO : thi implementation need overwriting protection

#define FLAG_TARGET_DIR  0x01
#define FLAG_TARGET_FILE 0x02

struct opt opts[] = {
	OPT('t',"--target-directory",FLAG_TARGET_DIR,"treat DESTINATION as destination directory"),
	OPT('T',"--no-target-directory",FLAG_TARGET_FILE,"treat DESTINATION as destionation file (NOTE : can only move one file with this option"),
};

const char *usage = "mv [OPTIONS] SOURCE... DESTINATION\n"
"or mv OPTION\n"
"move files and directories\n";

int ret = 0;

int move(const char *src,const char *dest){
	if(rename(src,dest) < 0){
		perror(src);
		ret = 1;
		return -1;
	}
	return 0;
}


int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));

	if(i + 2 > argc){
		error("missing argument");
		return 1;
	}

	if((flags & FLAG_TARGET_DIR) && (flags & FLAG_TARGET_FILE)){
		//FIXME : this error message don't look professional
		error("can't provide -T and -t at the same time");
	}

	char *dest = argv[argc-1];

	//automatic
	if(!(flags & FLAG_TARGET_DIR) && !(flags & FLAG_TARGET_FILE)){
		//let figure out ourself
		if(i < argc - 2 || dest[strlen(dest)-1] == '/'){
			flags |= FLAG_TARGET_DIR;
		} else {
			struct stat st;
			if(stat(dest,&st) < 0){
				flags |= FLAG_TARGET_FILE;
			} else if(S_ISDIR(st.st_mode)){
				flags |= FLAG_TARGET_DIR;
			} else {
				flags |= FLAG_TARGET_FILE;
			}
		}
	}

	//check our guesses + user input
	struct stat st;
	if(stat(dest,&st) < 0){
		if(flags & FLAG_TARGET_DIR){
			perror(dest);
			return 1;
		}
	} else if(flags & FLAG_TARGET_DIR){
		if(!S_ISDIR(st.st_mode)){
			errno = ENOTDIR;
			perror(dest);
			return 1;
		}
	} else {
		if(S_ISDIR(st.st_mode)){
			errno = EISDIR;
			perror(dest);
			return 1;
		}
	}
	if(i < argc - 2 && (flags & FLAG_TARGET_FILE)){
		error("can only copy one file with -T");
		return 1;
	}

	for(; i<argc-1; i++){
		//start by finding the dest path
		char dst[strlen(argv[i]) + strlen(dest) + 2];
		if(flags & FLAG_TARGET_FILE){
			strcpy(dst,dest);
		} else {
			sprintf(dst,"%s/%s",dest,argv[i]);
		}
		move(argv[i],dst);
	}

	return ret;
}
