#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <tutils.h>

#define FLAG_RECURSIVE 0x08

static int ret;

static opt_t opts[] = {
	OPT('R',"--recursive",FLAG_RECURSIVE,"operate on files and directories recursively"),
};

CMD(chmod, "chmod [OPTIONS]... MODE[,MODE]... FILES...\n"
"where MODE is [ugoa][-+=][perm...]\n"
"where perm is zero or more characters from rwxst\n"
"r for read\n"
"w for write\n"
"x for execute (or search on direcotories)\n"
"s for setuid or/and setgid bit\n"
"t for sticky bit (or restriction bit on directories\n"
"multiples MODE can be given separated by comma\n"
"change mode of files\n",
opts);

static int ch(const char *mode,const char *path){
	struct stat st;
	if(stat(path,&st) < 0){
		ret = 1;
		perror(path);
		return -1;
	}

	const char *m = mode;
	do{
		mode_t mask = S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID;
		switch(*m){
		case 'a':
			m++;
			break;
		case 'u':
			mask = S_IRWXU | S_ISUID;
			m++;
			break;
		case 'g':
			mask = S_IRWXG | S_ISGID;
			m++;
			break;
		case 'o':
			mask = S_IRWXO;
			m++;
			break;
		}
		mask |= S_ISVTX;
		int op = *m;
		m++;
		if(isdigit(op)){
			op = '=';
			m--;
		}
		if(op != '=' && op != '+' && op != '-'){
			error("invalid mode '%s'",mode);
		}
		mode_t mode = 0;
		if(isdigit(*m)){
			char *end;
			mode = (mode_t)strtol(m,&end,8);
			m = end;
		}
		else for(;;){
			switch(*m){
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
			default:
				goto finish_m;
			}
			m++;
		}
finish_m:
		mode &= mask;
		switch(op){
		case '=':
			st.st_mode = mode;
			break;
		case '+':
			st.st_mode |= mode;
			break;
		case '-':
			st.st_mode &= ~mode;
			break;
		}
		if(*m == ',')m++;
	}while(*m);

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
			ch(mode,full_path);
			if(!ret)break;
		}

		closedir(dir);
	}

	return 0;
}

static int chmod_main(int argc,char **argv){
	if(argc < 2){
		error("missing argument");
		return 1;
	}
	ret = 0;

	for(int i=1; i<argc; i++){	
		ch(argv[0],argv[i]);
	}
	return ret;
}
