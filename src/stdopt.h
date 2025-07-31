#ifndef STDOPT_H
#define STDOPT_H

#include <stdio.h>
#include <string.h>

//simple library for options


//simple macro used for things like --help or error

#define iprintf(...) fprintf(stderr,__VA_ARGS__)

#undef arraylen
#define arraylen(array) (sizeof(array) / sizeof(*array))

#ifdef __stanix__
#define UID "%ld"
#define GID "%ld"
#else
#define UID "%d"
#define GID "%d"
#endif

struct opt {
	char c;
	char *str;
	int flags;
	char **value;
	char *desc;
};

extern int flags;

int parse_arg(int argc,char **argv,struct opt *opt,size_t opt_count);
uid_t str2uid(const char *str);

#define OPT(pc,pstr,pflag,d) {.c = pc,.str = pstr,.flags = pflag,.desc = d}
#define OPTV(pc,pstr,pflag,val,d) {.c = pc,.str = pstr,.flags = pflag,.value = val,.desc = d}

void error(const char *fmt,...);
#endif
