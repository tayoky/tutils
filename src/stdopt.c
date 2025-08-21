#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pwd.h>
#include "stdopt.h"

int flags;
extern const char *usage;
const char *progname;

uid_t str2uid(const char *str){
	char *end;
	uid_t ret = (uid_t)strtol(str,&end,10);
	if(end == str){
		struct passwd *pwd = getpwnam(str);
		if(!pwd){
			return -1;
		}
		ret = pwd->pw_uid;
	}
	return ret;
}

void error(const char *fmt,...){
	va_list args;
	va_start(args,fmt);
	fprintf(stderr,"%s : ",progname ? progname : "tutils");
	vfprintf(stderr,fmt,args);
	va_end(args);
	fputc('\n',stderr);
}

void version(){
	printf("tutils 0.5.0\n");
	printf("see https://github.com/tayoky/tutils for last version\n");
	printf("wrote by tayoky\n");
}

static void help(struct opt *opts,size_t opts_count){
	//first find size for left col
	int size = 0;
	for(int i=0; i<opts_count; i++){
		size_t cur = 0;
		if(opts[i].str){
			cur += strlen(opts[i].str) + 1;
		}
		if(opts[i].c){
			cur += 3;
		}
		if(size < cur)size = cur;
	}

	printf("usage : %s",usage);
	for(int i=0; i<opts_count; i++){
		size_t cur = 0;
		if(opts[i].str){
			cur += strlen(opts[i].str) + 1;
		}
		if(opts[i].c){
			cur += 3;
		}
		if(opts[i].c){
			printf("-%c ",opts[i].c);
		}
		if(opts[i].str){
			printf("%s ",opts[i].str);
		}

		//align
		while(cur < size){
			putchar(' ');
			cur++;
		}
		printf(": %s\n",opts[i].desc);
	}
}

int parse_arg(int argc,char **argv,struct opt *opt,size_t opt_count){
	flags = 0;
	int i;
	progname = argv[0] ? strrchr(argv[0],'/') : NULL;
	if(progname){
		progname++;
	} else {
		progname = argv[0];
	}
	for(i=1; i<argc;i++){
		if(argv[i][0] != '-')break;
		if(argv[i][1] == '-'){
			//it's a long option
			//special case for --help and --version
			if(!strcmp("--help",argv[i])){
				help(opt,opt_count);
				exit(0);
			}
			if(!strcmp("--version",argv[i])){
				version();
				exit(0);
			}
			for(int j=0; j<opt_count; j++){
				if(opt[j].str && !strcmp(argv[i],opt[j].str)){
					flags |= opt[j].flags;
					if(opt[j].value){
						if(i == argc-1){
							error("expected argument after '%s'",argv[i]);
							exit(1);
						}
						i++;
						*opt[j].value = argv[i];
					}
					goto finish;
				}

			}
			error("unknow option '%s' (see --help)",argv[i]);
			exit(1);
		} else {
			//it's a short options
			int skip_next = 0;
			for(int l=1; argv[i][l]; l++){
				for(int j=0; j<opt_count; j++){
					if(opt[j].c == argv[i][l]){
						flags |= opt[j].flags;
						if(opt[j].value){
							if(i == argc-1){
								error("expected argument after '%c'",argv[i][l]);
								exit(1);
							}
							*opt[j].value = argv[i+1];
							skip_next = 1;
						}
					
	goto finish_short;
					}

				}
				error("unknow option '-%c' (see --help)",argv[i][l]);
				exit(1);
finish_short:
				continue;
			}
			if(skip_next)i++;
		}
finish:
		continue;
	}

	return i;
}
