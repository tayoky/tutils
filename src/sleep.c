#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <tutils.h>
#include <unistd.h>

CMD_NOPT(sleep, "sleep DURATION...",
	"or sleep OPTION\n"
	"Where DURATION is an integer that can be folowed by a suffix to choice the unit.\n"
	"s for second\n"
	"m for minute\n"
	"h for hour\n"
	"d for day\n"
	"If no suffix is provided assume seconds.\n"
	"Sleep will pause for the provided duration.\n"
	"If multiples durations are provided sleep will pause for their sum.\n");

static int sleep_main(int argc, char **argv) {
	if (argc < 1) {
		error(_("missing argument"));
		return 1;
	}

	long dur = 0;
	for (int i = 0; i < argc; i++) {
		if (argv[i][0] == '\0') {
			error(_("numeric argument required"));
			return 1;
		}
		char suffix;
		suffix = argv[i][strlen(argv[i]) - 1];
		if (isdigit(suffix)) {
			suffix = 's';
		} else {
			argv[i][strlen(argv[i]) - 1] = '\0';
		}

		char *end;
		unsigned long value = strtol(argv[i], &end, 10);
		if (argv[i] == end) {
			error(_("numeric argument required"));
			return 1;
		}

		switch (suffix) {
		case 's':
			break;
		case 'm':
			value *= 60;
			break;
		case 'h':
			value *= 3600;
			break;
		case 'd':
			value *= 24 * 3600;
			break;
		default:
			error(_("invalid suffix '%c'"), suffix);
			return 1;
		}

		dur += value;
	}

	if (sleep(dur) < 0) {
		perror("sleep");
		return 1;
	}
	return 0;
}
