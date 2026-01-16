#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <tutils.h>

#define FLAG_P 0x08
#define FLAG_L 0x10
static opt_t opts[] = {
	OPT('P',NULL,FLAG_P,"show the $PWD variable"),
	OPT('L',NULL,FLAG_L,"show the physical path without sysmlink"),
};

CMD(pwd, "pwd [-L]\n"
"or pwd -P\n"
"print the working directory\n"
"by default pwd behave like called with -L\n",
opts);

static int pwd_main(int argc,char **argv){
	if(flags & FLAG_P){
		char cwd[256];
		getcwd(cwd,256);
		printf("%s\n",cwd);
	} else {
		char *pwd = getenv("PWD");
		printf("%s\n",pwd);
	}
	return 0;
}
