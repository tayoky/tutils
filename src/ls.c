#include "stdopt.h"
#include <dirent.h>
#include <errno.h>

void help(){
	fprintf(stderr,"help !!!\n");
}

VERSION("beta v0.0.1")

int main(int argc,char **argv){
	int all = 0;
	ARGSTART
	case 'A':
		all = 1;
		break;
	case 'a':
		all = 2;
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

		printf("%s\n",name);
	}

	closedir(dir);
}
