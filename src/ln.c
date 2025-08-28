#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <libgen.h>
#include <dirent.h>
#include <fcntl.h>
#include "stdopt.h"

//TODO : this implementation need implicit . support

#define FLAG_TARGET_DIR  0x01
#define FLAG_TARGET_FILE 0x02
#define FLAG_FORCE       0x04
#define FLAG_INTERACTIVE 0x08

struct opt opts[] = {
	OPT('t',"--target-directory",FLAG_TARGET_DIR,"treat DESTINATION as destination directory"),
	OPT('T',"--no-target-directory",FLAG_TARGET_FILE,"treat DESTINATION as destination file (NOTE : can only move one file with this option"),
	OPT('i',"--interactive",FLAG_INTERACTIVE,"ask before overwriting old file"),
	OPT('f',"--force",FLAG_FORCE,"unlink destinations files if already exist"),
};

const char *usage = "ln [OPTIONS] SOURCE... DESTINATION\n"
"or ln OPTION\n"
"create hard link\n";

int ret = 0;

int ln(const char *src,const char *dest){
	struct stat src_st;
	if(stat(src,&src_st) < 0 && lstat(src,&src_st) < 0){
		perror(src);
		ret = 1;
		return -1;
	}

	if(S_ISDIR(src_st.st_mode)){
		errno = EISDIR;
		perror(src);
		ret = 1;
		return -1;
	}

	struct stat dest_st;
	if(stat(dest,&dest_st) >= 0){
		if(S_ISDIR(src_st.st_mode)){
			//can't overwrite dir
			errno = EISDIR;
			perror(dest);
			ret = 1;
			return -1;
		}
		//prompt before overwriting if needed
		if(flags & FLAG_INTERACTIVE){
			fprintf(stderr,"ln : overwrite '%s' ? [y/N] : ",dest);
			char buf[4096];
			fgets(buf,sizeof(buf),stdin);
			if(strcasecmp(buf,"y") && strcasecmp(buf,"yes")){
				ret = 1;
				return -1;
			}
		} else if(flags & FLAG_FORCE){
			if(unlink(dest) < 0){
				perror(dest);
				ret = 1;
				return -1;
			}
		} else {
			errno = EEXIST;
			perror(dest);
			ret = 1;
			return -1;
		}
	}

	if(link(src,dest) < 0){
		perror(dest);
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
		error("can only link one file with -T");
		return 1;
	}

	for(; i<argc-1; i++){
		//start by finding the dest path
		char dst[strlen(argv[i]) + strlen(dest) + 2];
		char src[strlen(argv[i]) + 1];
		strcpy(src,argv[i]);
		if(flags & FLAG_TARGET_FILE){
			strcpy(dst,dest);
		} else {
			sprintf(dst,"%s/%s",dest,basename(src));
		}
		ln(argv[i],dst);
	}

	return ret;
}
