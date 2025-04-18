#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "stdopt.h"

int ret = 0;
int hidden = 0;

int show(char *path,int depth){
	DIR *dir = opendir(path);
	if(!dir){
		iprintf("%s : %s\n",path,strerror(errno));
		ret = -1;
		return -1;
	}

	for(;;){
		struct dirent *entry = readdir(dir);
		if(!entry)break;

		//ignore . and ..
		if((!strcmp(entry->d_name,".")) || (!strcmp(entry->d_name,".."))) continue;

		//ignore hidden entry
		if(entry->d_name[0] == '.' && !hidden)continue;

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

void help(){
	iprintf("tree [-a]\n");
}

VERSION("v0.1.0")

int main(int argc,char **argv){
	ARGSTART
	case 'a':
		hidden = 1;
		break;
	ARGEND
	
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
