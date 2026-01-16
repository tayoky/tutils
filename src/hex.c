#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <tutils.h>

#define ESC "\033"
#define FLAG_SEEK   0x01
#define FLAG_LENGHT 0x02

static char *seek_str;
static char *lenght_str;
static size_t seek_value;
static size_t lenght_value;

static opt_t opts[] = {
	OPTV('s',"--seek",FLAG_SEEK,&seek_str,"seek to a specified location before dumping"),
	OPTV('l',"--length",FLAG_LENGHT,&lenght_str,"dump up to LENGTH byte"),
};

CMD(hex, "hex [OPTIONS] FILES\n"
"do an hexadecimal dump of files\n",
opts);

static size_t col = 8;
static int color = 1;

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
		printf("%02hhX ",line[i]);
	}
	for(size_t i=size;i<col;i++){
		printf("   ");
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

	if(flags & FLAG_SEEK){
		fseek(file,seek_value,SEEK_SET);
	}

	unsigned char buf[256];
	unsigned char line[256];
	size_t size;
	size_t line_len=0;
	size_t total = 0;
	while((size = fread(buf,1,sizeof(buf),file))){
		if((flags & FLAG_LENGHT) && total >= lenght_value)break;
		if((flags & FLAG_LENGHT) && size > lenght_value - total)size = lenght_value - total;
		total += size;
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

	printf(ESC"[0m");

	fclose(file);
	return 0;
}

static int hex_main(int argc,char **argv){
	if(argc < 1){
		error("missing argument");
		return 1;
	}
	if(flags & FLAG_SEEK){
		char *end;
		seek_value = strtol(seek_str,&end,0);
		if(end == seek_str){
			error("invalid value '%s'",seek_str);
			return 1;
		}
	}
	if(flags & FLAG_LENGHT){
		char *end;
		lenght_value = strtol(lenght_str,&end,0);
		if(end == lenght_str){
			error("invalid value '%s'",lenght_str);
			return 1;
		}
	}
	int ret = 0;
	for(int i=0;i<argc;i++){
		if(hex_dump(argv[i])){
			ret = 1;
		}
	}
	return ret;
}
