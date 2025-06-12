#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "stdopt.h"

void help(){
	iprintf("pwd : -[LP]\n");
	iprintf("-L : show the $PWD variable\n");
	iprintf("-P : show the pysical path without sysmlink\n");
	iprintf("by default, pwd behave like called with -L\n");
}

VERSION("v0.1.0");

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
