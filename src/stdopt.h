#ifndef STDOPT_H
#define STDOPT_H

#include <stdio.h>
#include <string.h>

//simple library for options


//simple macro used for things like --help or error

#define iprintf(...) fprintf(stderr,__VA_ARGS__)

#undef arraylen
#define arraylen(array) (sizeof(array) / sizeof(*array))

struct opt {
	char c;
	char *str;
	int flags;
	char **value;
};

extern int flags;

void parse_arg(int argc,char **argv,struct opt *opt,size_t opt_count);

#define OPT(pc,pstr,pflag) {.c = pc,.str = pstr,.flags = pflag}
#define OPTV(pc,pstr,pflag,val) {.c = pc,.str = pstr,.flags = pflag,.value = val}


#endif
