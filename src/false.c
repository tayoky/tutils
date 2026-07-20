#include <tutils.h>

CMD_NOPT(false, "false",
	"Exit with failures");

static int false_main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	return 1;
}
