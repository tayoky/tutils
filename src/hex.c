#include <stdio.h>
#include <ctype.h>
#include "stdopt.h"

#define ESC "\033"

struct opt opts[] = {
};

const char *usage = "hex FILES\n"
"do an hexadecimal dump of files\n";


size_t col = 8;
int color = 1;

static void print_color(char c){
	if(c){
		if(isprint(c)){
			printf(ESC"[1;32m");
		} else if(iscntrl(c)){
			printf(ESC"[1;33m");
		} else {
			printf(ESC"[1;31m");
		}
	} else {
		printf(ESC"[0m");
	}
}

static void print_line(unsigned char *line,size_t size){
	for(size_t i=0;i<size;i++){
		print_color(line[i]);
		printf("%02X ",line[i]);
	}

	for(size_t i=0;i<size;i++){
		print_color(line[i]);
		if(isprint(line[i])){
			putchar(line[i]);
		} else {
			putchar('.');
		}
	}

	putchar('\n');
}

static int hex_dump(char *path){
	FILE *file = fopen(path,"r");
	if(!file){
		perror(path);
		return 1;
	}

	unsigned char buf[256];
	unsigned char line[256];
	size_t size;
	size_t line_len=0;
	while((size = fread(buf,1,sizeof(buf),file))){
		for(size_t i=0;i<size;i++){
			line[line_len] = buf[i];
			line_len++;
			if(line_len >= col){
				print_line(line,line_len);
				line_len = 0;
			}
		}
	}

	if(line_len){
		print_line(line,line_len);
	}


	fclose(file);
	return 0;
}

int main(int argc,char **argv){
	parse_arg(argc,argv,opts,arraylen(opts));
	int ret = 0;
	size_t count = 0;
	for(int i=1;i<argc;i++){
		if(argv[i][0] == '-')continue;
		count++;
		if(hex_dump(argv[i])){
			ret = 1;
		}
	}

	if(!count){
		iprintf("no file specified\n");
	}
	return ret;
}
