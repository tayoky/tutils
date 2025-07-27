#include "stdopt.h"
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#ifdef __unix__
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pwd.h>
#endif

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


int to_tty = 0;

char **list = NULL;

size_t entry_count;

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
		if(flags &FLAG_LIST){
			printf("?????????? ");
		}
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
		} else if(S_ISCHR(info.st_mode)){
			printf(ESC"[1;33m");
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

	//are we wrinting to a tty ?
	to_tty = isatty(STDOUT_FILENO);
	if(to_tty < 0){
		to_tty = 0;
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

	//find cell_size and cell per line
	size_t cell_size = 1;
	size_t cell_per_line = 1;
	if(to_tty && !(flags & FLAG_LIST)){
		for(size_t i=0; i<entry_count; i++){
			if(strlen(list[i]) + 1 > cell_size){
				cell_size = strlen(list[i]) + 1;
			}
		}
#ifdef __unix__
		struct winsize win;
		if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&win) < 0){
			perror("ioctl");
		} else {
			cell_per_line = win.ws_col / cell_size;
		}
#endif
	}


	//print time !!
	for(size_t i=0; i<entry_count; i++){
		//print info first
		info(list[i]);
		printf("%s",list[i]);
		for(size_t j=strlen(list[i]); j<cell_size; j++)putchar(' ');
		if(to_tty)printf(ESC"[0m");
		if(i % cell_per_line == cell_per_line - 1 || i + 1 == entry_count)putchar('\n');
	}

	closedir(dir);
}
