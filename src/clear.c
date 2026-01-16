#include <stdio.h>
#include <tutils.h>

#define ESC "\033"

CMD_NOPT(clear, "clear [OPTION]\n"
"clear screen\n");

static int clear_main(int argc,char **argv){
	(void)argc;
	(void)argv;
	printf(ESC"[2J");
	printf(ESC"[H");
	return 0;
}
