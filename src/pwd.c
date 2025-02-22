#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "stdopt.h"

void help(){
	iprintf("pwd : -[LP]\n");
	iprintf("\n");
	iprintf("-L\n");
	iprintf("show the $PWD variable\n");
	iprintf("-P\n");
	iprintf("show the pysical path without sysmlink\n");
	iprintf("by default pwd behave like called with -L\n");
}

VERSION("beta v0.0.2");

int phys = 0;

int main(int argc,char **argv){

	ARGSTART
	case 'P':
		phys = 1;
		break;
	case 'L':
		phys = 0;
		break;
	ARGEND

	if(phys){
		char cwd[256];
		getcwd(cwd,256);
		printf("%s\n",cwd);
	} else {
		char *pwd = getenv("PWD");
		printf("%s\n",pwd);
	}
	return 0;
}
