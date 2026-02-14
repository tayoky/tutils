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
	int arg_type;
	void *value;
	char *desc;
} opt_t;

#define OPT_NOARG 0
#define OPT_STR   1
#define OPT_INT   2
#define OPT_SIZE  3
#define OPT_MODE  4

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

#define OPT(_c, _str, _flags, _desc) {.c = _c, .str = _str, .flags = _flags, .desc = _desc}
#define OPTARG(_c, _str, _flags, _arg_type, _ptr, _desc) {.c = _c, .str = _str, .flags = _flags, .arg_type = _arg_type, .value = _ptr, .desc = _desc}

#define OPTSTR(_c, _str, _flags, _ptr, _desc) OPTARG(_c, _str, _flags, OPT_STR, _ptr, _desc)
#define OPTINT(_c, _str, _flags, _ptr, _desc) OPTARG(_c, _str, _flags, OPT_INT, _ptr, _desc)
#define OPTSIZE(_c, _str, _flags, _ptr, _desc) OPTARG(_c, _str, _flags, OPT_SIZE, _ptr, _desc)
#define OPTMODE(_c, _str, _flags, _ptr, _desc) OPTARG(_c, _str, _flags, OPT_MODE, _ptr, _desc)

// compatibility with some old utils
#define OPTV(_c, _str, _flags, _ptr, _desc) OPTARG(_c, _str, _flags, OPT_STR, _ptr, _desc)

#define CMD(_name, _usage, _opts) static int _name ## _main(int argc, char **argv);\
	command_t _name ## _cmd = {.name = #_name, .usage = _usage, .options = _opts, .options_count = arraylen(_opts), .main = _name ## _main}
#define CMD_NOPT(_name, _usage) static int _name ## _main(int argc, char **argv);\
	command_t _name ## _cmd = {.name = #_name, .usage = _usage, .options = NULL, .options_count = 0, .main = _name ## _main}

void error(const char *fmt,...);
int glob_match(const char *glob, const char *str);

#define perror(str) error("%s : %s",str,strerror(errno));

#endif
