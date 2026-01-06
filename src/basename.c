#include <stdio.h>
#include <libgen.h>
#include "stdopt.h"

#define FLAG_MULTIPLE 0x08
#define FLAG_SUFFIX   0x10

char *suffix;

struct opt opts[] = {
	OPT('a',"--multiple",FLAG_MULTIPLE,"support multiple NAMEs in one command"),
	OPTV('s',"--suffix",FLAG_SUFFIX | FLAG_MULTIPLE,&suffix,"remove a trailing SUFFIX from file path, implies -a"),
};

const char *usage = "basename NAME [SUFFIX]\n"
"or basename OPTIONS... NAME...\n"
"strip directory name/suffix from files path\n";

void bname(char *path){
	path = basename(path);
	if(flags & FLAG_SUFFIX){
		if(strlen(path) >= strlen(suffix)){
			if(!strcmp(&path[strlen(path) - strlen(suffix)],suffix)){
				path[strlen(path) - strlen(suffix)] = '\0';
			}
		}
	}

	printf("%s\n",path);
}

int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));

	if(argc - 1 < 1){
		error("missing argument");
		return 1;
	}

	if(flags & FLAG_MULTIPLE){
		for(;i<argc;i++){
			bname(argv[i]);
		}
	} else {
		if(argc > 3){
			error("too much argument");
			return 1;
		}
		if(argc == 3){
			suffix = argv[2];
			flags |= FLAG_SUFFIX;
		}
		bname(argv[1]);
	}

	return 0;
}
