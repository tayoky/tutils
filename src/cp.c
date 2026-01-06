#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <libgen.h>
#include <dirent.h>
#include <fcntl.h>
#include "stdopt.h"

//TODO : this implementation need -HPL

#define FLAG_TARGET_DIR  0x01
#define FLAG_TARGET_FILE 0x02
#define FLAG_FORCE       0x04
#define FLAG_INTERACTIVE 0x08
#define FLAG_RECURSIVE   0x10

struct opt opts[] = {
	OPT('t',"--target-directory",FLAG_TARGET_DIR,"treat DESTINATION as destination directory"),
	OPT('T',"--no-target-directory",FLAG_TARGET_FILE,"treat DESTINATION as destination file (NOTE : can only move one file with this option"),
	OPT('i',"--interactive",FLAG_INTERACTIVE,"ask before overwriting old file"),
	OPT('f',"--force",FLAG_FORCE,"unlink destinations files if already exist"),
	OPT('r',"--recursive",FLAG_RECURSIVE,"copy directories and their content"),
};

const char *usage = "cp [OPTIONS] SOURCE... DESTINATION\n"
"or cp OPTION\n"
"copy files and directories\n";

int ret = 0;

int copy(const char *src,const char *dest,int cmdline){
	int src_fd = -1;
	int dest_fd = -1;
	struct stat src_st;
	if(stat(src,&src_st) < 0 && lstat(src,&src_st) < 0){
		perror(src);
		ret = 1;
		return -1;
	}

	if(S_ISDIR(src_st.st_mode) && !(flags & FLAG_RECURSIVE)){
		errno = EISDIR;
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
		if(S_ISDIR(src_st.st_mode)){
			//TODO : copy perm i guess ? 
		} else {
			//prompt before overwriting if needed
			if(flags & FLAG_INTERACTIVE){
				fprintf(stderr,"cp : overwrite '%s' ? [y/N] : ",dest);
				char buf[4096];
				fgets(buf,sizeof(buf),stdin);
				if(strcasecmp(buf,"y") && strcasecmp(buf,"yes")){
					ret = 1;
					return -1;
				}
			}
			dest_fd = open(dest,O_WRONLY | O_TRUNC);
			if(dest_fd < 0 && (flags & FLAG_FORCE)){
				unlink(dest);
				dest_fd = open(dest,O_WRONLY | O_TRUNC | O_CREAT);
			}
			if(dest_fd < 0){
				perror(dest);
				ret = 1;
				return -1;
			}
		}
	} else {
		if(S_ISDIR(src_st.st_mode)){
			mkdir(dest,src_st.st_mode);
		} else {
			dest_fd = open(dest,O_CREAT | O_TRUNC | O_WRONLY);
			if(!dest_fd){
				perror(dest);
				ret = 1;
				return -1;
			}
		}
	}
	
	chmod(dest,src_st.st_mode);

	if(S_ISDIR(src_st.st_mode)){
		//TODO : copy files
		DIR *dir = opendir(src);
		if(!dir){
			perror(src);
			ret = 1;
			return -1;
		}
		for(;;){
			struct dirent *ent = readdir(dir);
			if(!ent)break;
			if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))continue;
			char d[strlen(dest) + strlen(ent->d_name) + 2];
			char s[strlen(src) + strlen(ent->d_name) + 2];
			if(dest[strlen(dest)-1] == '/'){
				sprintf(d,"%s%s",dest,ent->d_name);
			} else {
				sprintf(d,"%s/%s",dest,ent->d_name);
			}
			if(src[strlen(src)-1] == '/'){
				sprintf(s,"%s%s",src,ent->d_name);
			} else {
				sprintf(s,"%s/%s",src,ent->d_name);
			}
			copy(s,d,0);
		}
		closedir(dir);
	} else {
		//copy content and meta
		src_fd = open(src,O_RDONLY);
		if(src_fd < 0){
			perror(src);
			close(dest_fd);
			ret = 1;
			return -1;
		}
		char buf[4096];
		for(;;){
			ssize_t r = read(src_fd,buf,sizeof(buf));
			if(!r)break;
			if(r < 0){
				perror(src);
				ret = 1;
				break;
			}
			ssize_t w = write(dest_fd,buf,r);
			if(w < 0){
				perror(dest);
				ret = 1;
				break;
			}
		}
		close(src_fd);
		close(dest_fd);
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
		char src[strlen(argv[i]) + 1];
		strcpy(src,argv[i]);
		if(flags & FLAG_TARGET_FILE){
			strcpy(dst,dest);
		} else {
			sprintf(dst,"%s/%s",dest,basename(src));
		}
		copy(argv[i],dst,1);
	}

	return ret;
}
