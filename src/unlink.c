#include <tutils.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

CMD_NOPT(unlink, "unlink FILE\nor unlink OPTION\n"
"unlink a symlink/hardlink or remove a file\n");


static int unlink_main(int argc,char **argv){
	if(argc < 2){
		error("missing argument");
		return 1;
	}
	if(argc > 2){
		error("too many arguments");
		return 1;
	}	

	if(unlink(argv[1])){
		perror(argv[1]);
		return 1;
	}
	return 0;
}
