#include "stdopt.h"
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define ESC "\033"

void help(){
	iprintf("ls [-laAU] [DIRECTORY]\n");
	iprintf("list all files in a directory\n");
	iprintf("-A\nshow all files except . and ..\n");
	iprintf("-a\nshow all files (including . and ..)\n");
	iprintf("-l\nshow one file/directory per line\n");
	iprintf("-U\nshow in directory order without sorting\n");
}

VERSION("v0.1.0")

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

void color(char *path){
	struct stat info;
	if(lstat(path,&info)){
		iprintf("%s : %s\n",path,strerror(errno));
		exit(1);
	}
	if(info.st_mode & S_IXUSR){
		printf(ESC"[1;32m");
	}
	if(S_ISDIR(info.st_mode)){
		printf(ESC"[1;34m");
	}
}

int alpha_sort(const void *e1,const void *e2){
	return strcmp(*(const char **)e1,*(const char **)e2);
}

int main(int argc,char **argv){
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

	int all = 0;
	int directory_order = 0;
	ARGSTART
	case 'A':
		all = 1;
		break;
	case 'a':
		all = 2;
		break;
	case 'l':
		//everythings on a separate line
		column = 1;
		break;
	case 'U':
		directory_order = 1;
		break;
	ARGEND
	
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
		return -1;
	}

	struct dirent *entry;

	for(;;){
		entry = readdir(dir);
		if(!entry){
			break;
		}

		char *name = entry->d_name;

		//check for hidden entry
		if((!all) && name[0] == '.'){
			continue;
		}

		//check for . and ..
		if(all < 2){
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
	if(!directory_order){
		qsort(list,entry_count,sizeof(char *),alpha_sort);
	}
#endif

	//determinates column sizes
	int *column_size = malloc(column);
	for(int i=0; i<column; i++){
		int size = 0;
		for(int j=i; j<entry_count; j+= column){
			if(strlen(list[j]) > size){
				size = strlen(list[j]);
			}
		}
		column_size[i] = size;
	}
	
	//print time !!
	int column_index = 0;
	for(int i=0; i<entry_count; i++){
		if(column_index >= column){
			column_index = 0;
			printf("\n");
		} else if (i){
			printf(" ");
		}
		//color only for tty
		if(to_tty)color(list[i]);
		printf("%*s",-column_size[column_index],list[i]);
		if(to_tty)printf(ESC"[0m");
		column_index++;
	}
	printf("\n");

	closedir(dir);
}
