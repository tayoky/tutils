#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc,char **argv){
	int ret = 0;
	for(int i=1; i<argc;i++){
		FILE *file;
		if(!strcmp(argv[i],"-")){
			file = stdin;
		} else {
			file = fopen(argv[i],"r");
		}
		//handle error
		if(file == NULL){
			fprintf(stderr,"%s : %s \n",argv[i],strerror(errno));
			ret = -1;
			continue;
		}

		for(;;){
			int c = fgetc(file);
			if(feof(file)){
				break;
			}
			putchar(c);
		}
		//don't close stdin
		if(file != stdin){
			fclose(file);
		}
	}
	return ret;
}

