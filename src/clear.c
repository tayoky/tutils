#include <stdio.h>
#include "stdopt.h"

#define ESC "\033"

const char *usage = "clear [OPTION]\n"
"clear screen\n";

int main(int argc,char **argv){
	parse_arg(argc,argv,NULL,0);
	printf(ESC"[2J");
	printf(ESC"[H");
	return 0;
}
