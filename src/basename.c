#include <stdio.h>
#include <libgen.h>
#include <tutils.h>

#define FLAG_MULTIPLE 0x08
#define FLAG_SUFFIX   0x10

static char *suffix;

static opt_t opts[] = {
	OPT('a',"--multiple",FLAG_MULTIPLE,"support multiple NAME in one command"),
	OPTV('s',"--suffix",FLAG_SUFFIX | FLAG_MULTIPLE,&suffix,"remove a trailing SUFFIX from file path, implies -a"),
};

CMD(basename, "basename NAME [SUFFIX]\n"
"or basename OPTIONS... NAME...\n"
"strip directory name/suffix from files path\n",
opts);

static void bname(char *path){
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

static int basename_main(int argc,char **argv){
	if(argc < 1){
		error("missing argument");
		return 1;
	}

	if(flags & FLAG_MULTIPLE){
		for(int i=0;i<argc;i++){
			bname(argv[i]);
		}
	} else {
		if(argc > 3){
			error("too many arguments");
			return 1;
		}
		if(argc == 3){
			suffix = argv[1];
			flags |= FLAG_SUFFIX;
		}
		bname(argv[0]);
	}

	return 0;
}
