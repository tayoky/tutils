#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <tutils.h>


static opt_t opts[] = {
};

CMD(readlink, "readlink [OPTIONS] FILES...\n"
"print value of symbolic link\n",
opts);

static int readlink_main(int argc,char **argv){
	if(argc < 1){
		error("missing argument");
		return 1;
	}
	int ret = 0;
	char buf[PATH_MAX + 1];
	for(int i=0;i<argc;i++){
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
