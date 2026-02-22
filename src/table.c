#include <tutils.h>

#define CMD_LIST \
	X(bracket)\
	X(basename) \
	X(cat) \
	X(chmod) \
	X(chown) \
	X(clear) \
	X(cp) \
	X(dd) \
	X(dirname) \
	X(echo) \
	X(false) \
	X(find) \
	X(head) \
	X(hex) \
	X(id) \
	X(ln) \
	X(ls) \
	X(mkdir) \
	X(mv) \
	X(pwd) \
	X(readlink) \
	X(rm) \
	X(rmdir) \
	X(seq) \
	X(sleep) \
	X(stat) \
	X(test) \
	X(touch) \
	X(true) \
	X(unlink) \
	X(wc) \
	X(whoami) \
	X(xargs) \
	X(yes) \

#define X(name) extern command_t name ## _cmd;
CMD_LIST
#undef X

#define X(name) &name ## _cmd,
command_t *commands[] = {
	CMD_LIST
};
#undef X

size_t commands_count = arraylen(commands);
