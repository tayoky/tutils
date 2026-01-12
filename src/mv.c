#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <libgen.h>
#include "stdopt.h"

//TODO : this implementation need overwriting protection

#define FLAG_TARGET_DIR  0x01
#define FLAG_TARGET_FILE 0x02
#define FLAG_FORCE       0x04
#define FLAG_INTERACTIVE 0x08

struct opt opts[] = {
	OPT('t',"--target-directory",FLAG_TARGET_DIR,"treat DESTINATION as destination directory"),
	OPT('T',"--no-target-directory",FLAG_TARGET_FILE,"treat DESTINATION as destination file (NOTE : can only move one file with this option"),
	OPT('i',"--interactive",FLAG_INTERACTIVE,"always ask before overwriting old file"),
	OPT('f',"--force",FLAG_FORCE,"always overwrite old file without asking"),
};

const char *usage = "mv [OPTIONS] SOURCE... DESTINATION\n"
"or mv OPTION\n"
"move files and directories\n";

int ret = 0;

int move(const char *src,const char *dest){
	struct stat src_st;
	if(stat(src,&src_st) < 0 && lstat(src,&src_st) < 0){
		perror(src);
		ret = 1;
		return -1;
	}
	struct stat dest_st;
	if(stat(dest,&dest_st) >= 0){
		if(S_ISDIR(src_st.st_mode) != S_ISDIR(dest_st.st_mode)){
			//can't overwrite file with dir and "vice-versa"
			errno = S_ISDIR(dest_st.st_mode) ? EISDIR : ENOTDIR;
			perror(dest);
			ret = 1;
			return -1;
		}
		//prompt before overwriting if needed
		if(!(flags & FLAG_FORCE) && ((flags & FLAG_INTERACTIVE) || (0/*TODO : can't write*/ && isatty(STDIN_FILENO) == 1))){
			fprintf(stderr,"mv : overwrite '%s' ? [y/N] : ",dest);
			char buf[4096];
			fgets(buf,sizeof(buf),stdin);
			if(strcasecmp(buf,"y") && strcasecmp(buf,"yes")){
				ret = 1;
				return -1;
			}
		}
	}
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
		error("cannot provide -T and -t simultaneously");
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
		char src[strlen(argv[i]) + 1];
		strcpy(src,argv[i]);
		if(flags & FLAG_TARGET_FILE){
			strcpy(dst,dest);
		} else {
			sprintf(dst,"%s/%s",dest,basename(src));
		}
		move(argv[i],dst);
	}

	return ret;
}
