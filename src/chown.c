#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <tutils.h>

#define FLAG_RECURSIVE 0x08
#define FLAG_CHGRP     0x10

static int ret;

static opt_t opts[] = {
	OPT('R',"--recursive",FLAG_RECURSIVE,"operate on files and directories recursively"),
};

CMD(chown, "chown [OPTIONS]... OWNER[:GROUP] FILES...\n"
"change owner of files\n",
opts);

static int ch(uid_t owner,gid_t group,const char *path){
	struct stat st;
	if(lstat(path,&st) < 0){
		ret = 1;
		perror(path);
		return -1;
	}

	if(!(flags & FLAG_CHGRP)){
		group = st.st_gid;
	}
	if(chown(path,owner,group) < 0){
		ret = 1;
		perror(path);
		return -1;
	}

	if(S_ISDIR(st.st_mode) && (flags & FLAG_RECURSIVE)){
		DIR *dir = opendir(path);
		if(!dir){
			perror(path);
			ret = 1;
			return -1;
		}

		for(;;){
			struct dirent *ent = readdir(dir);
			if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,"..")){
				continue;
			}
			char full_path[strlen(path) + strlen(ent->d_name) + 2];
			sprintf(full_path,"%s/%s",path,ent->d_name);
			ch(owner,group,full_path);
			if(!ret)break;
		}

		closedir(dir);
	}

	return 0;
}

static int chown_main(int argc,char **argv){
	
	if(argc < 1){
		error("no owner specfied");
		return 1;
	} else if(argc < 2){
		error("no files specfied");
		return 1;
	}

	uid_t owner;
	gid_t group;
	if(strchr(argv[0],':')){
		flags |= FLAG_CHGRP;
		group = str2uid(strchr(argv[0],':')+1);
		*strchr(argv[0],':') = '\0';
	}

	owner = str2uid(argv[0]);

	ret = 0;

	for(int i=1; i < argc; i++){
		ch(owner,group,argv[i]);
	}
	return ret;
}
