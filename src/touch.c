#include <stdio.h>
#include "stdopt.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int create = 1;
int change_modify = 1;

VERSION("beta 0.0.1")

void help(){
	iprintf("touch -[ca]\n");
	iprintf("-a\nchange only access time\n");
	iprintf("-c\ndon't create any file\n");
}

#define DMODE S_IRUSR |S_IWUSR |S_IRGRP |S_IWGRP

int main(int argc,char **argv){
	ARGSTART
	case 'c':
		create = 0;
		break;
	case 'a':
		change_modify = 0;
		break;
	ARGEND

	//make the flags
	int flags = O_RDWR ;
	if(change_modify){
		flags = O_RDONLY;
	}
	if(create){
		flags |= O_CREAT;
	}

	int ret = 0;

	for(int i=1; i<argc; i++){
		if(argv[i][0] == '-')continue;
		int fd = open(argv[i],flags,DMODE);
		if(fd < 0){
			ret = -1;
			iprintf("%s : %s\n",argv[i],strerror(errno));
			continue;
		}
		close(fd);
	}
		
	return 0;
}
