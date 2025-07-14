#include <stdio.h>
#include <string.h>
#include "stdopt.h"


const char *usage = "yes [STRING]\nor yes OPTION\n"
"repeatedly print a string to stdout\n";

int main(int argc,char **argv){
	parse_arg(argc,argv,NULL,0);

  for(;;) {
    if(argc > 1) {
      for(int c = 1; c < argc; c++) printf("%s ", argv[c]);
    } else {
      printf("y");
    }
    puts("");
  }
}
