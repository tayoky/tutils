#include "stdopt.h"
#include <dirent.h>

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
	
	DIR *dir = opendir(".");

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
