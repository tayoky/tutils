#include <tutils.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

CMD_NOPT(unlink, "unlink FILE\nor unlink OPTION\n"
"unlink a symlink/hardlink or remove a file\n");


static int unlink_main(int argc,char **argv){
	if(argc < 1){
		error("missing argument");
		return 1;
	}
	if(argc > 1){
		error("too many arguments");
		return 1;
	}	

	if(unlink(argv[0])){
		perror(argv[0]);
		return 1;
	}
	return 0;
}
