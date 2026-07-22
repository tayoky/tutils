#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <tutils.h>
#include <unistd.h>

CMD_NOPT(unlink, "unlink FILE\nor unlink OPTION",
	"Unlink a symlink/hardlink or remove a file.");


static int unlink_main(int argc, char **argv) {
	if (argc < 1) {
		error(_("missing argument"));
		return 1;
	}
	if (argc > 1) {
		error(_("too many arguments"));
		return 1;
	}

	if (unlink(argv[0])) {
		perror(argv[0]);
		return 1;
	}
	return 0;
}
