#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include "stdopt.h"

const char *usage = "whoami [OPTION]\n"
"print current username or uid\n";

int main(int argc,char **argv){
	parse_arg(argc,argv,NULL,0);
	struct passwd *pwd = getpwuid(geteuid());
	if(pwd){
		puts(pwd->pw_name);
	} else {
		puts("unknow");
		return 1;
	}
}
