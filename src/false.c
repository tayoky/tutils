#include <tutils.h>

CMD_NOPT(false, "false\n"
"exit with failure\n");

static int false_main(int argc, char **argv){
	(void)argc;
	(void)argv;
	return 1;
}
