#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <tutils.h>

//TODO : this impmementation is missing xflag cbs conv and count

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

CMD_NOPT(dd, "dd [OPERANDS]\n"
"or dd OPTION\n"
"copy and convert files\n"
"by default dd read stdin and output to stdout with 512 bytes blocks\n"
"operand are of the form OPERAND=VALUE ...\n"
"valid operand are\n"
"bs=BYTES  : use input and output blocks of size BYTES (overide ibs and obs)\n"
"conv=CONV : precise convert option separated by comma\n"
"count=N   : copy only up to N input block\n"
"ibs=BYTES : use input blocks of size BYTES\n"
"if=FILE   : read from FILE instead of stdin\n"
"obs=BYTES : use output blocks of size BYTES\n"
"of=FILE   : write to FILE instead of stdin\n"
"skip=N (or iseek=N) : skip N input block\n"
"seek=N (or oseek=N) : skip N output block\n");


static size_t str2int(const char *str){
	char *end;
	size_t i = strtoul(str,&end,0);
	if(end == str)goto invalid;

	if(!*end)return i;


	//suffix handling

	//special case for some one char prefix
	if(!end[1]){
		switch(*end){
		case 'c':
		case 'B':
			return i;
		case 'w':
			return i * 2;
		case 'b':
			return i * 512;
		}
	}

	static char prefix[] = "KMGTPEZYRQ";
	size_t base = 1024;
	if(end[1]){
		if(!strcmp(&end[1],"B")){
			base = 1000;
		} else if(strcmp(&end[1],"iB")){
			goto invalid;
		}
	}

	if(!strchr(prefix,*end)){
		goto invalid;
	}

	size_t power = strchr(prefix,*end) - prefix + 1;
	while(power > 0){
		i *= base;
		power--;
	}

	return i;
invalid:
	error("invalid value for operand '%s'",str);
	exit(1);
}

static size_t read_block(int fd,char *buf,size_t bs){
	ssize_t r;
	size_t total = 0;
	while(bs && (r = read(fd,buf,bs)) > 0){
		buf   += r;
		bs    -= r;
		total += r;
	}
	if(r < 0){
		perror("read");
		return 1;
	}
	return total;
}

static size_t read_by_blocks(int fd,char *buf,size_t count,size_t bs){
	size_t total = 0;
	size_t r;
	while(count && (r = read_block(fd,buf,min(count,bs))) > 0){
		buf   += r;
		count -= r;
		total += r;
		if(r < bs)break;
	}
	return total;
}

static int dd_main(int argc, char **argv){
	char *in_name  = NULL;
	char *out_name = NULL;
	size_t ibs = 512;
	size_t obs = 512;
	size_t iseek = 0;
	size_t oseek = 0;
	size_t count = SIZE_MAX;
	int oflags = O_WRONLY | O_TRUNC | O_CREAT;

	for(int i=0; i<argc; i++){
		if(!strchr(argv[i],'=')){
			error("invalid operand '%s'",argv[i]);
			return 1;
		}
		char *operand = argv[i];
		char *value = strchr(operand,'=');
		*(value++) = '\0';

#define OP(str) !strcmp(str,operand)
		if(OP("bs")){
			ibs = obs = str2int(value);
			if(!obs)error("invalid number '%s'",value);
			continue;
		}
		if(OP("count")){
			count = str2int(value);
			continue;
		}
		if(OP("conv")){
			size_t conv_count=1;
			char *ptr = value;
			while(strchr(ptr,',')){
				conv_count++;
				ptr = strchr(ptr,',');
				*ptr = '\0';
				ptr++;
			}
			ptr = value;
			while(conv_count-- > 0){
				if(!strcmp(ptr,"notrunc")){
					oflags &= ~O_TRUNC;
				} else if(!strcmp(ptr,"excl")){
					oflags |= O_EXCL;
				} else {
					error("invalid conv option '%s'",ptr);
					return 1;
				}
				ptr += strlen(ptr) + 1;
			}
			continue;
		}
		if(OP("ibs")){
			ibs = str2int(value);
			if(!ibs)error("invalid number '%s'",value);
			continue;
		}
		if(OP("if")){
			in_name = value;
			continue;
		}
		if(OP("obs")){
			obs = str2int(value);
			if(!obs)error("invalid number '%s'",value);
			continue;
		}
		if(OP("of")){
			out_name = value;
			continue;
		}
		if(OP("seek") || OP("oseek")){
			oseek = str2int(value);
			continue;
		}
		if(OP("skip") || OP("iseek")){
			iseek = str2int(value);
			continue;
		}

		*(--value) = '=';
		error("unknow operand '%s' (see --help)",operand);
		return 1;
	}

	int in,out;

	if(in_name){
		in = open(in_name,O_RDONLY);
		if(in < 0){
			perror(in_name);
			return 1;
		}
	} else {
		in = STDIN_FILENO;
	}

	if(out_name){
		out = open(out_name,oflags,0777);
		if(out < 0){
			perror(out_name);
			return 1;
		}
	} else {
		out = STDOUT_FILENO;
	}

	//TODO : fallback for iseek on char devices (eg tty,...), skip instead of seeking
	if(iseek)lseek(in ,ibs * iseek,SEEK_SET);
	if(oseek)lseek(out,obs * oseek,SEEK_SET);

	size_t bs = max(ibs,obs);
	char *buf = malloc(bs);
	if(!buf){
		perror("malloc");
		return 1;
	}

	size_t total_w = 0;
	size_t total_r = 0;
	int ret = 0;
	for(;;){
		size_t content = read_by_blocks(in,buf,min(bs,ibs * count),ibs);
		total_r += content;
		int eof = content < bs;

		char *wbuf = buf;
		while(content > 0){
			ssize_t w = write(out,wbuf,min(content,obs));
			if(w < 0){
				perror("write");
				ret = 1;
				goto end;
			}
			if(!w){
				//uh ???
				error("suspicious write fail (write returned 0)");
				ret = 1;
				goto end;
			}
			total_w += w;
			content -= w;
			wbuf += w;
		}
		count -= bs / ibs;
		if(eof)break;
	}

end:
	printf("%zd+%zd records in\n",total_r/ibs,total_r%ibs);
	printf("%zd+%zd records out\n",total_w/obs,total_w%obs);
	printf("%zd bytes copied\n",min(total_r,total_w));
	free(buf);
	if(in != STDIN_FILENO)close(in);
	if(out != STDOUT_FILENO)close(out);

	return 0;
}
