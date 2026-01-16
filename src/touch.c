#include <stdio.h>
#include <tutils.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define FLAG_NO_CREATE 0x08
#define FLAG_ACCESS    0x10
#define FLAG_MODIFY    0x20

static opt_t opts[] = {
	OPT('a',NULL,FLAG_ACCESS,"only change acces time"),
	OPT('m',NULL,FLAG_MODIFY,"only change modify time"),
	OPT('c',"--no-create",FLAG_NO_CREATE,"don't create any file"),
};

CMD(touch, "touch [-cam] FILES...\n"
"create files/update access and modification times\n",
opts);

#define DMODE S_IRUSR | S_IWUSR

static int touch_main(int argc,char **argv){
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
	if(argc < 1){
		error("missing argument");
		return 1;
	}

	for(int i=0; i<argc; i++){
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
