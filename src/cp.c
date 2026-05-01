#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <libgen.h>
#include <dirent.h>
#include <fcntl.h>
#include <tutils.h>

//TODO : this implementation need -HPL

#define FLAG_TARGET_DIR  0x01
#define FLAG_TARGET_FILE 0x02
#define FLAG_FORCE       0x04
#define FLAG_INTERACTIVE 0x08
#define FLAG_RECURSIVE   0x10

static opt_t opts[] = {
	OPT('t',"--target-directory",FLAG_TARGET_DIR,"treat DESTINATION as destination directory"),
	OPT('T',"--no-target-directory",FLAG_TARGET_FILE,"treat DESTINATION as destination file (NOTE : can only move one file with this option"),
	OPT('i',"--interactive",FLAG_INTERACTIVE,"ask before overwriting old file"),
	OPT('f',"--force",FLAG_FORCE,"unlink destinations files if already exist"),
	OPT('r',"--recursive",FLAG_RECURSIVE,"copy directories and their content"),
};

CMD(cp, "cp [OPTIONS] SOURCE... DESTINATION\n"
"or cp OPTION\n"
"copy files and directories\n",
opts);

static opt_t mv_opts[] = {
	OPT('t',"--target-directory",FLAG_TARGET_DIR,"treat DESTINATION as destination directory"),
	OPT('T',"--no-target-directory",FLAG_TARGET_FILE,"treat DESTINATION as destination file (NOTE : can only move one file with this option"),
	OPT('i',"--interactive",FLAG_INTERACTIVE,"ask before overwriting old file"),
	OPT('f',"--force",FLAG_FORCE,"unlink destinations files if already exist"),
};

CMD(mv, "mv [OPTIONS] SOURCE... DESTINATION\n"
"or mv OPTION\n"
"move files and directories\n",
mv_opts);

static int ret = 0;
static int is_mv = 0;

static int copy(const char *src,const char *dest,int cmdline){
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
		if (src_st.st_dev == dest_st.st_dev && src_st.st_ino == dest_st.st_ino) {
			error("'%s' and '%s' are the same file", src, dest);
			ret = 1;
			return -1;
		}
		if(S_ISDIR(src_st.st_mode)){
			//TODO : copy perm i guess ? 
		} else {
			// prompt before overwriting if needed
			if(flags & FLAG_INTERACTIVE){
				fprintf(stderr,"%s : overwrite '%s' ? [y/N] : ", is_mv ? "mv" : "cp", dest);
				char buf[4096];
				fgets(buf,sizeof(buf),stdin);
				if(strcasecmp(buf,"y") && strcasecmp(buf,"yes")){
					ret = 1;
					return -1;
				}
			}
		}
	}

	if (is_mv) {
		// first try rename
		// FIXME : move this around
		if (rename(src, dest) >= 0) {
			return 0;
		}
	}

	if(S_ISDIR(src_st.st_mode)){
		mkdir(dest,src_st.st_mode);
	} else {
		dest_fd = open(dest,O_CREAT|O_WRONLY | O_TRUNC);
		if(dest_fd < 0 && (flags & FLAG_FORCE)){
			unlink(dest);
			dest_fd = open(dest,O_CREAT | O_TRUNC | O_WRONLY);
		}
		if(dest_fd < 0){
			perror(dest);
			ret = 1;
			return -1;
		}
	}
	
	chmod(dest,src_st.st_mode);

	if(S_ISDIR(src_st.st_mode)){
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
		if (is_mv) {
			// we rmdir only when we are sure the content have been copied
			rmdir(src);
		}
	} else {
		// copy content and meta
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

		if (is_mv) {
			// we unlink only when we are sure the content have been copied
			unlink(src);
		}
	}

	return 0;
}


static int cp_main(int argc,char **argv){
	is_mv = !strcmp(progname, "mv");

	if(argc < 2){
		error("missing argument");
		return 1;
	}

	if((flags & FLAG_TARGET_DIR) && (flags & FLAG_TARGET_FILE)){
		//FIXME : this error message don't look professional
		error("cannot provide -T and -t at the same time");
	}

	char *dest = argv[argc-1];

	// automatic
	if(!(flags & FLAG_TARGET_DIR) && !(flags & FLAG_TARGET_FILE)){
		// let figure out ourself
		if(argc > 2 || dest[strlen(dest)-1] == '/'){
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

	// check our guesses + user input
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
	if(argc > 2 && (flags & FLAG_TARGET_FILE)){
		if (is_mv) {
			error("can only move one file with -T");
		} else {
			error("can only copy one file with -T");
		}
		return 1;
	}

	for(int i=0; i<argc-1; i++){
		// start by finding the dest path
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

static int mv_main(int argc, char **argv) {
	// mv does not need -r
	flags |= FLAG_RECURSIVE;
	return cp_main(argc, argv);
}
