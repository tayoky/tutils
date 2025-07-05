#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include "stdopt.h"

#define FLAG_RECURSIVE 0x08
#define FLAG_CHGRP     0x10

int ret;

struct opt opts[] = {
	OPT('R',"--recursive",FLAG_RECURSIVE,"operate on files and directories recursively"),
};

const char *usage = "chown [OPTIONS]... OWNER[:GROUP] FILES...\n"
"change owner of files\n";

uid_t str2uid(const char *str){
	return atoi(str);
}

int ch(uid_t owner,gid_t group,const char *path){
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

int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));
	
	if(i >= argc){
		error("no owner specfied");
		return 1;
	} else if(i + 1 >= argc){
		error("no files specfied");
		return 1;
	}

	uid_t owner;
	gid_t group;
	if(strchr(argv[i],':')){
		flags |= FLAG_CHGRP;
		group = str2uid(strchr(argv[i],':')+1);
		*strchr(argv[i],':') = '\0';
	}

	owner = str2uid(argv[i]);

	i++;

	ret = 0;

	while(i < argc){
		ch(owner,group,argv[i]);
		i++;
	}
	return ret;
}
