#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "stdopt.h"

#define FLAG_TABS   0x08
#define FLAG_ENDS   0x10
#define FLAG_NUMBER 0x20
#define FLAG_NOPRNT 0x40

struct opt opts[] = {
	OPT('A',"--show-all",FLAG_NOPRNT | FLAG_ENDS | FLAG_TABS,"equivalent to -vET"),
	OPT('e',NULL,FLAG_NOPRNT | FLAG_ENDS,"equivalent to -vE"),
	OPT('E',"--show-ends",FLAG_ENDS,"display $ at end of each line"),
	OPT('n',"--number",FLAG_NUMBER,"number each line"),
	OPT('t',NULL,FLAG_NOPRNT | FLAG_TABS,"equivalent to -vT"),
	OPT('T',"--show-tabs",FLAG_TABS,"display tabs characters as ^I"),
	OPT('u',NULL,0,"ignored (just kept here for posix)"),
	OPT('v',"--show-noprinting",FLAG_NOPRNT,"display non printable characters with ^ notation (except for NL and TAB)"),
};

int ret;

const char *usage = "cat [FILES] ...\n"
"concatenate files and print to stdout\n"
"special file name \"-\" is equivalent to stdin\n"
"if no files is specified stdin is used by default\n";

void cat(const char *path){
	FILE *file;
	//"-" is stdin
	if(!strcmp(path,"-")){
		file = stdin;
	} else {
		file = fopen(path,"r");
	}

	//handle error
	if(file == NULL){
		perror(path);
		ret = 1;
		return;
	}

	int c;
	int prev = EOF;
	int line = 0;
	while((c = fgetc(file)) != EOF){
		if((flags & FLAG_NUMBER) && (prev == EOF || prev == '\n'))printf("%6d  ",line++);
		if(c == '\n' && (flags & FLAG_ENDS)){
			putchar('$');
		} else if(c == '\t' && (flags & FLAG_TABS)){
			putchar('^');
			putchar('\t' - 1 + 'A');
			prev = c;
			continue;
		}

		if(!isprint(c) && c != '\n' && c != '\t'){
			putchar('^');
			putchar(c - 1 + 'A');
		} else {
			putchar(c);
		}
		prev = c;
	}



	//don't close stdin
	if(file != stdin){
		fclose(file);
	}
}

int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));
	ret = 0;
	//if nothing stdin by default
	if(argc ==  i){
		cat("-");
	} else {
		for(;i<argc;i++){
			cat(argv[i]);
		}
	}
	
	return ret;
}
