#include <stdio.h>
#include <stdlib.h>
#include "stdopt.h"

const char *usage = "whoami [OPTION]\n"
"print current username or uid\n";

int main(int argc,char **argv){
	parse_arg(argc,argv,NULL,0);

	char *username = getenv("USER");
	if(username == NULL){
		fprintf(stderr,"whoami : $USER environement variable not set\n");
		return 1;
	}
	printf("%s\n",username);
	return 0;
}
