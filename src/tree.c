#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "stdopt.h"

#define FLAG_ALL 0x80

struct opt opts[] = {
	OPT('a',NULL,FLAG_ALL,"show hidden files and directories"),
};

const char *usage = "tree [-a] [DIRECTORY]\n"
"recursively show content of directories\n";

int ret = 0;

int show(char *path,int depth){
	DIR *dir = opendir(path);
	if(!dir){
		iprintf("%s : %s\n",path,strerror(errno));
		ret = 1;
		return -1;
	}

	for(;;){
		struct dirent *entry = readdir(dir);
		if(!entry)break;

		//ignore . and ..
		if((!strcmp(entry->d_name,".")) || (!strcmp(entry->d_name,".."))) continue;

		//ignore hidden entry
		if(entry->d_name[0] == '.' && !(flags & FLAG_ALL))continue;

		//print identation
		for(int i=0; i<depth-1; i++){
			printf("│   ");
		}
		printf("├───");

		printf("%s",entry->d_name);

		//find the full name
		char *full_name = malloc(strlen(path) + strlen(entry->d_name) + 2);
		strcpy(full_name,path);
		if(path[strlen(path)-1] != '/'){
			strcat(full_name,"/");
		}
		strcat(full_name,entry->d_name);

		struct stat st;
		stat(full_name,&st);
		if(S_ISDIR(st.st_mode)){
			printf("/\n");
			show(full_name,depth + 1);
		} else {
			putchar('\n');
		}

		free(full_name);
	}

	closedir(dir);

	return 0;
}

int main(int argc,char **argv){
	parse_arg(argc,argv,opts,arraylen(opts));
	
	int count = 0;

	for(int i=1; i<argc; i++){
		if(argv[i][0] != '-'){
			printf("%s\n",argv[i]);
			show(argv[i],1);
			count++;
		}
	}

	if(!count){
		printf(".\n");
		show(".",1);
	}

	return ret;
}
