#ifndef TUTILS_H
#define TUTILS_H

#include <stdio.h>
#include <string.h>
#include <errno.h>

#undef arraylen
#define arraylen(array) (sizeof(array) / sizeof(*array))

#ifdef __stanix__
#define FUID "%ld"
#define FGID "%ld"
#else
#define FUID "%d"
#define FGID "%d"
#endif

typedef struct opt {
	char c;
	char *str;
	int flags;
	char **value;
	char *desc;
} opt_t;

typedef struct command {
	const char *usage;
	const char *name;
	int (*main)(int argc, char **argv);
	opt_t *options;
	size_t options_count;
} command_t;

extern int flags;
extern command_t *commands[];
extern size_t commands_count;
extern char *progname;

uid_t str2uid(const char *str);

#define OPT(pc,pstr,pflag,d) {.c = pc,.str = pstr,.flags = pflag,.desc = d}
#define OPTV(pc,pstr,pflag,val,d) {.c = pc,.str = pstr,.flags = pflag,.value = val,.desc = d}
#define CMD(_name, _usage, _opts) static int _name ## _main(int argc, char **argv);\
	command_t _name ## _cmd = {.name = #_name, .usage = _usage, .options = _opts, .options_count = arraylen(_opts), .main = _name ## _main}
#define CMD_NOPT(_name, _usage) static int _name ## _main(int argc, char **argv);\
	command_t _name ## _cmd = {.name = #_name, .usage = _usage, .options = NULL, .options_count = 0, .main = _name ## _main}

void error(const char *fmt,...);
int glob_match(const char *glob, const char *str);

#define perror(str) error("%s : %s",str,strerror(errno));

#endif
