#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <tutils.h>

CMD_NOPT(sleep, "sleep DURATION...\n"
"or sleep OPTION\n"
"where DURATION is an integer that can be folowed by a suffix to choice the unit\n"
"s for second\n"
"m for minute\n"
"h for hour\n"
"d for day\n"
"if no suffix is provided assume seconds\n"
"sleep will pause for the provided duration\n"
"if multiples durations are provided sleep will pause for their sum\n");

static int sleep_main(int argc,char **argv){
	if(argc < 1){
		error("missing argument");
		return 1;
	}

	long dur = 0;
	for(int i=0; i<argc; i++){
		if(argv[i][0] == '\0'){
			error("numeric argument required");
			return 1;
		}
		char suffix;
		suffix = argv[i][strlen(argv[i])-1];
		if(isdigit(suffix)){
			suffix = 's';
		} else {
			argv[i][strlen(argv[i])-1] = '\0';
		}

		char *end;
		unsigned long value = strtol(argv[i],NULL,10);
		if(argv[i] == end){
			error("numeric argument required");
			return 1;
		}

		switch(suffix){
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
			error("invalid suffix '%c'",suffix);
			return 1;
		}

		dur += value;
	}

	if(sleep(dur) < 0){
		perror("sleep");
		return 1;
	}
	return 0;
}
