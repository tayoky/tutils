#include "stdopt.h"
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#define ESC "\033"
#define FLAG_ALMOST  0x08
#define FLAG_ALL     0x10
#define FLAG_NO_SORT 0x20
#define FLAG_LIST    0x40

struct opt opts[] = {
	OPT('a',"--all",FLAG_ALL | FLAG_ALMOST,"show all files (including . and ..)"),
	OPT('A',"--almost-all",FLAG_ALMOST,"show all files except . and .."),
	OPT('U',NULL,FLAG_NO_SORT,"show in directory order without sorting"),
	OPT('l',"--list",FLAG_LIST,"show one file/directory per line"),
};

const char *usage = "ls [-laAU] [DIRECTORY]\n"
"list files in a directory\n";


int column = 5;
int to_tty = 0;

char **list = NULL;

int entry_count;

void list_add(char *str){
	if(!list){
		list = malloc(1);
	}
	entry_count++;
	list = realloc(list,entry_count * sizeof(char *));
	list[entry_count-1] = str;
}

void info(char *path){
	struct stat info;
	if(lstat(path,&info)){
		iprintf("%s : %s\n",path,strerror(errno));
		exit(1);
	}

	//show mode if needed
	if(flags & FLAG_LIST){
		if(S_ISLNK(info.st_mode)){
			putchar('l');
		} else if(S_ISDIR(info.st_mode)){
			putchar('d');
		} else {
			putchar('-');
		}
#define MODEC(mode,c) if(info.st_mode & mode )putchar(c); \
	else putchar('-');
		MODEC(S_IRUSR,'r');
		MODEC(S_IWUSR,'w');
		MODEC(S_IXUSR,'x');
		MODEC(S_IRGRP,'r');
		MODEC(S_IWGRP,'w');
		MODEC(S_IXGRP,'x');
		MODEC(S_IROTH,'r');
		MODEC(S_IWOTH,'w');
		MODEC(S_IXOTH,'x');
#undef MODEC
		putchar(' ');

		//owner name
		struct passwd *pwd = getpwuid(info.st_uid);
		if(pwd){
			printf("%s ",pwd->pw_name);
		} else {
#ifdef __stanix__
			printf("%ld ",info.st_uid);
#else
			printf("%d ",info.st_uid);
#endif
		}
	
		printf("%4ld ",info.st_size);
	}
	
	//color only for tty
	if(to_tty){
		if(S_ISLNK(info.st_mode)){
			printf(ESC"[1;36m");
		} else if(S_ISDIR(info.st_mode)){
			printf(ESC"[1;34m");
		} else if(info.st_mode & S_IXUSR){
			printf(ESC"[1;32m");
		}
	}
}

int alpha_sort(const void *e1,const void *e2){
	return strcmp(*(const char **)e1,*(const char **)e2);
}

int main(int argc,char **argv){
	parse_arg(argc,argv,opts,arraylen(opts));

	if(flags & FLAG_LIST){
		column = 1;
	}

	//are we wrinting to a tty ?
	to_tty = isatty(STDOUT_FILENO);
	if(to_tty < 0){
		to_tty = 0;
	}

	//when writing to a not tty
	//only one output per line
	if(!to_tty){
		column = 1;
	}

	
	//get the dir to open
	char *dirpath = ".";
	for(int i=1;i<argc;i++){
		if(argv[i][0] != '-'){
			dirpath = argv[i];
			break;
		}
	}
	
	DIR *dir = opendir(dirpath);
	chdir(dirpath);

	if(dir == NULL){
		iprintf("%s : %s\n",dirpath,strerror(errno));
		return 1;
	}

	struct dirent *entry;

	for(;;){
		entry = readdir(dir);
		if(!entry){
			break;
		}

		char *name = entry->d_name;

		//check for hidden entry
		if(!(flags & FLAG_ALMOST) && name[0] == '.'){
			continue;
		}

		//check for . and ..
		if(!(flags & FLAG_ALL)){
			if(!strcmp(name,".")){
				continue;
			}
			if(!strcmp(name,"..")){
				continue;
			}	
		}

		list_add(name);
	}

#ifndef NO_QSORT
	//sort time ?
	if(!(flags & FLAG_NO_SORT)){
		qsort(list,entry_count,sizeof(char *),alpha_sort);
	}
#endif

	//determinates column sizes
	int *column_size = malloc(column);
	if(!(flags & FLAG_LIST))
	for(int i=0; i<column; i++){
		int size = 0;
		for(int j=i; j<entry_count; j+= column){
			if(strlen(list[j]) > size){
				size = strlen(list[j]);
			}
		}
		column_size[i] = size;
	}
	else memset(column_size,0,column * sizeof(int));
	
	//print time !!
	int column_index = 0;
	for(int i=0; i<entry_count; i++){
		if(column_index >= column){
			column_index = 0;
			printf("\n");
		} else if (i){
			printf(" ");
		}
		//print info first
		info(list[i]);
		printf("%*s",-column_size[column_index],list[i]);
		if(to_tty)printf(ESC"[0m");
		column_index++;
	}
	printf("\n");

	closedir(dir);
}
