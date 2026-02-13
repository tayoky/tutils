#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <tutils.h>

#define FLAG_VERBOSE 0x04
#define FLAG_PARENT  0x08

static int ret = 0;
static mode_t mode = S_IRWXU;
static char *m = NULL;

static opt_t opts[] = {
	OPT('p',"--parents",FLAG_PARENT,"make parents directories if needed and ignore if any directory already exist"),
	OPTV('m',"--mode",0,&m,"change mode of the news directories"),
	OPT('v',"--verbose",FLAG_VERBOSE,"print a message for each created directory"),
};


CMD(mkdir, "mkdir [-m MODE] [-pv] DIRECTORIES ...\n"
"create directories\n",
opts);

static void make_dir(const char *path){
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
		if(mkdir(parents[i],mode) < 0){
			//ignore aready exist error when parent mode
			if(errno == EEXIST && (flags & FLAG_PARENT)){
				continue;
			}
			perror(parents[i]);
			ret = 1;
			return;
		} else {
			if(flags & FLAG_VERBOSE)printf("mkdir : created directory '%s'\n",parents[i]);
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
	if(flags & FLAG_VERBOSE)printf("mkdir : created directory '%s'\n",path);
}

static int mkdir_main(int argc, char **argv){
	if(m){
		//TODO : parse m in others format
		char *end;
		mode = strtol(m,&end,8);
		if(end == m){
			error("invalid mode '%s'\n",m);
		}
	}

	if(argc < 1){
		error("missing argument");
		return 1;
	}

	for(int i=0; i<argc; i++){
		make_dir(argv[i]);
	}
	
	return ret;
}
