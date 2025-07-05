#include <stdio.h>
#include <stdarg.h>
#include "stdopt.h"

int flags;

void error(const char *fmt,...){
	va_list args;
	va_start(args,fmt);
	vfprintf(stderr,fmt,args);
	va_end(args);
	fputc('\n',stderr);
}

int parse_arg(int argc,char **argv,struct opt *opt,size_t opt_count){
	flags = 0;
	int i;
	for(i=1; i<argc;i++){
		if(argv[i][0] != '-')break;
		if(argv[i][1] == '-'){
			//it's a long option
			for(int j=0; j<opt_count; j++){
				if(opt[j].str && !strcmp(argv[i],opt[j].str)){
					flags |= opt[j].flags;
					if(opt[j].value){
						if(i == argc-1){
							error("expected argument after '%s'",argv[i]);
						}
						i++;
						*opt[j].value = argv[i];
					}
					goto finish;
				}

			}
			error("unknow option '%s' (see --help)",argv[i]);
		} else {
			//it's a short options
			for(int l=1; argv[i][l]; l++){
				for(int j=0; j<opt_count; j++){
					if(opt[j].c == argv[i][l]){
						flags |= opt[j].flags;
						goto finish_short;
					}

				}
				error("unknow option '-%c' (see --help)",argv[i][l]);
finish_short:
				continue;
			}
		}
finish:
		continue;
	}

	return i;
}
