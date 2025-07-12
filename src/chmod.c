#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include "stdopt.h"

#define FLAG_RECURSIVE 0x08
#define FLAG_ADD       0x10
#define FLAG_SUB       0x20
#define FLAG_SET       0x40

int ret;

struct opt opts[] = {
	OPT('R',"--recursive",FLAG_RECURSIVE,"operate on files and directories recursively"),
};

const char *usage = "chown [OPTIONS]... MODE[,MODE]... FILES...\n"
"where MODE is [ugoa][-+=][perm...]\n"
"where perm is zero or more characters from rwxst\n"
"r for read\n"
"w for write\n"
"x for execute (or search on direcotories)\n"
"s for setuid or/and setgid bit\n"
"t for sticky bit (or restriction bit on directories\n"
"multiples MODE can be given separated by comma\n"
"change mode of files\n";

uid_t str2uid(const char *str){
	return atoi(str);
}

int ch(mode_t set,mode_t add,mode_t sub,const char *path){
	struct stat st;
	if(stat(path,&st) < 0){
		ret = 1;
		perror(path);
		return -1;
	}

	if(flags & FLAG_SET){
		st.st_mode = set;
	}
	if(flags & FLAG_ADD){
		st.st_mode |= add;
	}
	if(flags & FLAG_SUB){
		st.st_mode &= ~sub;
	}

	if(chmod(path,st.st_mode) < 0){
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
			ch(set,add,sub,full_path);
			if(!ret)break;
		}

		closedir(dir);
	}

	return 0;
}

int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));
	
	if(i >= argc){
		error("no mode specfied");
		return 1;
	} else if(i + 1 >= argc){
		error("no files specfied");
		return 1;
	}

	mode_t set,add,sub = 0;
	for(int j=0;; j++){
		mode_t mask = S_IRWXU | S_IRWXG | S_IRWXO | S_ISVTX | S_ISUID | S_ISGID;
		switch(argv[i][j]){
		case 'a':
			j++;
			break;
		case 'u':
			mask = S_IRWXU | S_ISUID;
			j++;
			break;
		case 'g':
			mask = S_IRWXG | S_ISGID;
			j++;
			break;
		case 'o':
			mask = S_IRWXO;
			j++;
			break;
		}

		mode_t *to_set;
		switch(argv[i][j]){
		case '+':
			flags |= FLAG_ADD;
			to_set = &add;
			break;
		case '-':
			flags |= FLAG_SUB;
			to_set = &sub;
			break;
		case '=':
			flags |= FLAG_SET;
			to_set = &set;
			break;
		default:
			error("invalid mode : '%s'",argv[i]);
			return 1;
		}
		j++;

		mode_t mode = 0;
		for(;argv[i][j] && argv[i][j] != ',';j++){
			switch(argv[i][j]){
			case 'r':
				mode |= S_IRUSR | S_IRGRP | S_IROTH;
				break;
			case 'w':
				mode |= S_IWUSR | S_IWGRP | S_IWOTH;
				break;
			case 'x':
				mode |= S_IXUSR | S_IXGRP | S_IXOTH;
				break;
			case 's':
				mode |= S_ISUID | S_ISGID;
				break;
			case 't':
				mode |= S_ISVTX;
				break;
			case '+':
				*to_set |= mode & mask;
				flags |= FLAG_ADD;
				to_set = &add;
				break;
			case '-':
				*to_set |= mode & mask;
				flags |= FLAG_SUB;
				to_set = &sub;
				break;
			case '=':
				*to_set |= mode & mask;
				flags |= FLAG_SET;
				to_set = &set;
				break;

			default:
				error("invalid mode : '%s'",argv[i]);
				return 1;
			}
		}
		*to_set |= mode & mask;
		if(argv[i][j] == ','){
			j++;
		} else {
			break;
		}
	}


	i++;

	ret = 0;

	while(i < argc){
		ch(set,add,sub,argv[i]);
		i++;
	}
	return ret;
}
