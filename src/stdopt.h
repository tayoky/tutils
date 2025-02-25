#ifndef STDOPT_H
#define STDOPT_H

#include <stdio.h>
#include <string.h>

//simple header only library for options

void help();
void version();

static char verbose_mode = 0;

#define ARGSTART for(int j=0;j<argc;j++){ \
	if(argv[j][0] == '-'){ \
		if(!strcmp(argv[j],"--help")){\
			help();\
			return 0;\
		}\
		if(!strcmp(argv[j],"--version")){\
			version();\
			return 0;\
		}\
		switch(argv[j][1]){ \
			case 'h':\
				help();\
				return 0;\
				break;\
			case 'v':\
				version();\
				return 0;\
				break;

#define ARGEND 		default:\
				fprintf(stderr,"-%c : invalid option (see -h)\n",argv[j][1]);\
				return -1;\
			}\
		}\
	}

#define VERSION(ver) void version(){\
	fprintf(stderr,"Tayoky's core utils 0.0.3\n");\
	fprintf(stderr,"%s\n",ver);\
	fprintf(stderr,"see https://github.com/tayoky/tutils for last version\n");\
}

//simple macro used for things like --help or error

#define iprintf(...) fprintf(stderr,__VA_ARGS__)

#endif
