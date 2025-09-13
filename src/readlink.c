#include <unistd.h>
#include <stdio.h>
#include "stdopt.h"


struct opt opts[] = {
};

const char *usage = "readlink [OPTIONS] FILES...\n"
"print value of symbolic link\n";

int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));
	if(argc == i){
		error("missing argument");
		return 1;
	}
	int ret = 0;
	char buf[PATH_MAX + 1];
	for(;i<argc;i++){
		ssize_t size = readlink(argv[i],buf,sizeof(buf)-1);
		if(size < 0){
			ret = 1;
			continue;
		}
		buf[size] = '\0';
		puts(buf);
	}
	return ret;
}
