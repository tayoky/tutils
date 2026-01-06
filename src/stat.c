#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include "stdopt.h"

//identifier might vary accros oses
#ifdef __stanix__
#define UI "%lu"
#define GI "%lu"
#else
#define UI "%u"
#define GI "%u"
#endif

#define FLAG_LINK   0x08 //folow links
#define FLAG_FS     0x10
#define FLAG_FORMAT 0x20
#define FLAG_PRINTF 0x40
#define FLAG_TERSE  0x80

char *fmt = "  File : %n\n"
"Size   : %s  Blocks : %b  IO Block : %B  %F\n"
"Device : %Hd,%Ld  Inode : %i  Links : %h\n"
"Access : (%a/%A)  Uid : %U(%u)  Gid : %G(%g)\n"
"Access : %x\n"
"Modify : %y\n"
"Change : %z\n"
"Birth  : %w\n";

int ret;

struct opt opts[] = {
	OPT('L',"--dereference",FLAG_LINK,"follow links"),
	OPT('f',"--file-system",FLAG_FS,"display file system info instead of file info"),
	OPT(0,"--cached",0,"ignored"),
	OPTV('c',"--format",FLAG_FORMAT,&fmt,"use specified format instead of the default"),
	OPTV(0,"--printf",FLAG_PRINTF,&fmt,"like --format but interpret \\ use \\n if you want a newline"),
	OPT('t',"--terse",FLAG_TERSE,"print info in terse form"),
};

const char *usage = "stat [OPTIONS]... FILE...\n"
"display file or file system information\n";

void print_time(time_t t){
	char *str = ctime(&t);
	*strrchr(str,'\n') = '\0';
	printf("%s",str);
}

void do_stat(const char *path){
	struct stat st;
	int r;
	if(flags & FLAG_LINK){
		r = stat(path,&st);
	} else {
		r = lstat(path,&st);
	}
	if(r < 0){
		perror(path);
		ret = 1;
		return ;
	}

	char *f = fmt;
	while(*f){
	switch(*f){
	case '\\':
		if(flags & FLAG_PRINTF || !(flags & FLAG_FORMAT)){
			f++;
			switch(*f){
			case 'n':
				putchar('\n');
				break;
			case 't':
				putchar('\t');
				break;
			case 'e':
				putchar('\033');
				break;
			case '\0':
				f--;
				break;
			default:
				putchar(*f);
				break;
			}
		} else {
			putchar('\\');
		}
		break;
	case '%':
		f++;
		switch(*f){
		case '%':
			putchar('%');
			break;
		case 'a':
			printf("%#o",st.st_mode & 07777);
			break;
		case 'b':
			printf("%ld",st.st_blocks);
			break;
		case 'B':
			printf("%d",st.st_blksize);
			break;
		case 'd':
			printf("%ld",st.st_dev);
			break;
		case 'D':
			printf("%lx",st.st_dev);
			break;
		case 'f':
			printf("%x",st.st_mode);
			break;
		case 'F':
			//TODO : file type
			break;
		case 'g':
			printf(GI,st.st_gid);
			break;
		case 'G':
			//TODO group name
			printf(GI,st.st_gid);
			break;
		case 'h':
			printf("%u",st.st_nlink);
			break;
		case 'i':
			printf("%ld",st.st_ino);
			break;
		case 'm':
			//TODO : mount point
			//can this be implemented and portable ?
			printf("mount-point");
			break;
		case 'n':
			printf("%s",path);
			break;
		case 'N':
			//TODO : proper deference
			printf("\"%s\"",path);
			break;
		case 's':
			printf("%ld",st.st_size);
			break;
		case 'r':
			printf("%ld",st.st_rdev);
			break;
		case 'R':
			printf("%lx",st.st_rdev);
			break;
		case 't':
			printf("%x",major(st.st_rdev));
			break;
		case 'T':
			printf("%x",minor(st.st_rdev));
			break;
		case 'u':
			printf(UI,st.st_uid);
			break;
		case 'U':;
			struct passwd *pwd = getpwuid(st.st_uid);
			if(pwd){
				printf("%s",pwd->pw_name);
			} else {
				printf(UI,st.st_uid);
			}
			break;
		case 'w':
			putchar('-');
			break;
		case 'W':
			putchar('0');
			break;
		case 'x':
			print_time(st.st_atime);
			break;
		case 'X':
			printf("%ld",st.st_atime);
			break;
		case 'y':
			print_time(st.st_mtime);
			break;
		case 'Y':
			printf("%ld",st.st_mtime);
			break;
		case 'z':
			print_time(st.st_ctime);
			break;
		case 'Z':
			printf("%ld",st.st_ctime);
			break;
		case 'H':
			f++;
			switch (*f) {
			case 'd':
				printf("%d",major(st.st_dev));
				break;
			case 'r':
				printf("%d",major(st.st_rdev));
				break;
			default:
				putchar('H');
				putchar(*f);
				break;
			}
			break;
		case 'L':
			f++;
			switch (*f) {
			case 'd':
				printf("%d",minor(st.st_dev));
				break;
			case 'r':
				printf("%d",minor(st.st_rdev));
				break;
			default:
				putchar('L');
				putchar(*f);
				break;
			}
			break;
		}
		break;
	default:
		putchar(*f);
		break;
	}
	f++;
	}

	if(flags & FLAG_FORMAT){
		putchar('\n');
	}
}

int main(int argc,char **argv){
	int i = parse_arg(argc,argv,opts,arraylen(opts));

	if(i == argc){
		error("missing argument");
		return 1;
	}

	if(flags & FLAG_TERSE){
		fmt = "%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %W %o %C\n";
	}

	ret = 0;
	for(;i < argc; i++){
		do_stat(argv[i]);
	}
	return ret;
}
