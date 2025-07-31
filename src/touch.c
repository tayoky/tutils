#include <stdio.h>
#include "stdopt.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#define FLAG_NO_CREATE 0x08
#define FLAG_ACCESS    0x10
#define FLAG_MODIFY    0x20

struct opt opts[] = {
	OPT('a',NULL,FLAG_ACCESS,"only change acces time"),
	OPT('m',NULL,FLAG_MODIFY,"only change modify time"),
	OPT('c',"--no-create",FLAG_NO_CREATE,"don't create any file"),
};

const char *usage = "touch [-cam] FILES...\n"
"create files/update access and modification times\n";

#define DMODE S_IRUSR |S_IWUSR

int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));

	if(!(flags & (FLAG_ACCESS | FLAG_MODIFY))){
		flags |= FLAG_ACCESS | FLAG_MODIFY;
	}

	//make the flags
	int f;
	if((flags & FLAG_ACCESS) && (flags & FLAG_MODIFY)){
		f = O_RDWR;
	} else {
		if(flags & FLAG_ACCESS){
			f = O_RDONLY;
		} else {
			f = O_WRONLY;
		}
	}
	if(!(flags & FLAG_NO_CREATE)){
		f  |= O_CREAT;
	}

	int ret = 0;
	if(i == argc){
		error("missing argument");
		return 1;
	}

	for(; i<argc; i++){
		if(argv[i][0] == '-')continue;
		int fd = open(argv[i],f,DMODE);
		if(fd < 0){
			ret = 1;
			perror(argv[i]);
			continue;
		}
		close(fd);
	}

		
	return ret;
}
