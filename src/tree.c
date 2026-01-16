#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <tutils.h>

#define FLAG_ALL 0x80

static opt_t opts[] = {
	OPT('a',NULL,FLAG_ALL,"show hidden files and directories"),
};

CMD(tree, "tree [-a] [DIRECTORY]\n"
"recursively show content of directories\n",
opts);

static int ret = 0;

static int show(char *path,int depth){
	DIR *dir = opendir(path);
	if(!dir){
		perror(path);
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

static int tree_main(int argc,char **argv){
	if (argc < 1) {
		printf(".\n");
		show(".",1);
	} else {
		for(int i=0; i<argc; i++){
			printf("%s\n",argv[i]);
			show(argv[i],1);
		}
	}


	return ret;
}
