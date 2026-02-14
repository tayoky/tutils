#include <tutils.h>

CMD_NOPT(true, "true\n"
"exit with success\n");

static int true_main(int argc, char **argv){
	(void)argc;
	(void)argv;
	return 0;
}
