#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <tutils.h>

#define FLAG_TABS   0x08
#define FLAG_ENDS   0x10
#define FLAG_NUMBER 0x20
#define FLAG_NOPRNT 0x40
#define FLAG_BYTE   0x80

static opt_t opts[] = {
	OPT('A',"--show-all",FLAG_NOPRNT | FLAG_ENDS | FLAG_TABS | FLAG_BYTE,"equivalent to -vET"),
	OPT('e',NULL,FLAG_NOPRNT | FLAG_ENDS | FLAG_BYTE,"equivalent to -vE"),
	OPT('E',"--show-ends",FLAG_ENDS | FLAG_BYTE,"display $ at end of each line"),
	OPT('n',"--number",FLAG_NUMBER | FLAG_BYTE,"number each line"),
	OPT('t',NULL,FLAG_NOPRNT | FLAG_TABS | FLAG_BYTE,"equivalent to -vT"),
	OPT('T',"--show-tabs",FLAG_TABS | FLAG_BYTE,"display tabs characters as ^I"),
	OPT('u',NULL,FLAG_BYTE,"one byte at time (slow)"),
	OPT('v',"--show-noprinting",FLAG_NOPRNT | FLAG_BYTE,"display non printable characters with ^ notation (except for NL and TAB)"),
};

static int ret;

CMD(cat, "cat [FILES] ...\n"
"concatenate files and print to stdout\n"
"special file name \"-\" is equivalent to stdin\n"
"if no files is specified stdin is used by default\n",
opts);

static void cat(const char *path){
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

	if(flags & FLAG_BYTE){
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
	} else {
		char buffer[4096];
		size_t size;
		while((size = read(fileno(file),buffer,sizeof(buffer)))){
			write(STDOUT_FILENO,buffer,size);
		}
	}



	//don't close stdin
	if(file != stdin){
		fclose(file);
	}
}

static int cat_main(int argc,char **argv){
	ret = 0;
	//if nothing stdin by default
	if(argc < 1){
		cat("-");
	} else {
		for(int i=0;i<argc;i++){
			cat(argv[i]);
		}
	}
	
	return ret;
}
