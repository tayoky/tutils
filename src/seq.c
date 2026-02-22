#include <stdlib.h>
#include <stdio.h>
#include <tutils.h>

#define ESC "\033"

CMD_NOPT(seq, "seq [FIRST [INCREMENT]] LAST\n"
"print number from FIRST to LAST\n");

static int seq_main(int argc,char **argv){
	if (argc < 1) {
		error("missing argument");
		return 1;
	}
	if (argc > 3) {
		error("too many arguments");
		return 1;
	}
	double args[3];
	for (int i=0; i<argc; i++) {
		char *end;
		args[i] = strtof(argv[i], &end);
		if (argv[i] == end || *end) {
			error("invalid doubleing point number : '%s'", argv[i]);
			return 1;
		}
	}

	double start     = 1;
	double end       = 0;
	double increment = 1;

	end = args[argc - 1];
	if (argc > 1) {
		start = args[0];
	}
	if (argc > 2) {
		increment = args[1];
		if (increment == 0) {
			error("invalid incremrent value : '0'");
			return 1;
		}
	}

	double current = start;
	if (increment > 0) {
		// acount for imprecision
		while (end - current > -0.00001) {
			printf("%g\n", current);
			current += increment;
		}
	} else {
		// acount for imprecision
		while (end - current < 0.00001) {
			printf("%g\n", current);
			current += increment;
		}
	}
	return 0;
}
