#include <stdio.h>
#include <stdlib.h>
#include <tutils.h>

#define FLAG_LINES 0x08
#define FLAG_BYTES 0x10
#define FLAG_QUIET 0x20

static char *lines;
static char *bytes;
static int ret;
static long line;
static long byte;

static opt_t opts[] = {
	OPTV('n',"--lines",FLAG_LINES,&lines,"print the first NUM lines instead of 10 lines"),
	OPTV('c',"--bytes",FLAG_BYTES,&bytes,"the number of bytes to copy from the start"),
	OPT('q',"--quiet",FLAG_QUIET,"never print filename header"),
};

CMD(head, "head [OPTIONS]... [FILES]...\n"
"print the first 10 lines of a file\n"
"the special filename '-' is treat as stdin\n"
"if no files is specified, '-' is used by default\n",
opts);

static void head(const char *path){
	FILE *file;
	if(!strcmp(path,"-")){
		file = stdin;
	} else {
		file = fopen(path,"r");
	}

	if(!(flags & FLAG_QUIET)){
		printf("%s:\n",path);
	}

	if(!file){
		perror(path);
		ret = 1;
		return;
	}

	char buf[4096];
	if(flags & FLAG_LINES){
		size_t size;
		size_t found = 0;
		if(found  >= line)goto finish;
		while((size = fread(buf,1,sizeof(buf),file))){
			char *ptr = buf;
			while(memchr(ptr,'\n',size)){
				size_t line_len = (char *)memchr(ptr,'\n',size) - ptr + 1;
				fwrite(ptr,line_len,1,stdout);
				ptr += line_len;
				size -= line_len;
				found++;
				if(found >= line)goto finish;
			}
			fwrite(ptr,size,1,stdout);
		}
	} else {
		size_t size;
		size_t total = 0;
		while((size = fread(buf,1,sizeof(buf),file))){
			if(size + total > byte){
				fwrite(buf,byte - total,1,stdout);
				break;
			}
			total += size;
			fwrite(buf,size,1,stdout);
		}
	}

finish:
	if(file != stdin)
	fclose(file);
}

static int head_main(int argc,char **argv){
	//activate quiet mode for 0 or 1 file
	if(argc < 2){
		flags |= FLAG_QUIET;
	}

	if(flags & FLAG_LINES){
		char *ptr;
		line = strtol(lines,&ptr,10);
		if(ptr == lines || line < 0){
			error("positive numeric argument required");
			return 1;
		}
	} else if(flags & FLAG_BYTES){
		char *ptr;
		byte= strtol(bytes,&ptr,10);
		if(ptr == bytes|| byte < 0){
			error("positive numeric argument required");
			return 1;
		}
	} else {
		flags |= FLAG_LINES;
		line = 10;
	}

	ret = 0;
	if(argc < 1){
		head("-");
	} else {
		for(int i=0; i < argc; i++){
			head(argv[i]);
		}
	}

	return ret;

}
