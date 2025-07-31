#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include "stdopt.h"

#define FLAG_GROUPS 0x01
#define FLAG_GROUP  0x02
#define FLAG_USER   0x04
#define FLAG_REAL   0x08
#define FLAG_NAME   0x10

int ret;

struct opt options[] = {
	OPT('u',"--user"  ,FLAG_USER  ,"print only effective user id"),
	OPT('g',"--group" ,FLAG_GROUP ,"print only effective group id"),
	OPT('G',"--groups",FLAG_GROUPS,"print only real, effective and supplementary groups id"),
	OPT('r',"--real"  ,FLAG_REAL  ,"print real id instead of effective"),
	OPT('n',"--name"  ,FLAG_NAME  ,"print the name (string) instead of an id (integer)"),
};

const char *usage = "print user identity\n";

//TODO : support for suplementary groups + groups name
void print_info(uid_t uid){
	struct passwd *pwd = getpwuid(uid);
	if(!pwd){
		perror("getpwuid");
		ret = 1;
		return;
	}
	if(flags & FLAG_USER){
		if(flags & FLAG_NAME){
			printf("%s\n",pwd->pw_name);
		} else {
			printf(UID"\n",pwd->pw_uid);
		}
	} else if(flags & FLAG_GROUP){
		printf(GID"\n",pwd->pw_gid);
	} else if(flags & FLAG_GROUPS){
		printf("unsupported\n");
	} else {
		printf("uid="UID"(%s) gid="GID"("GID")\n",pwd->pw_uid,pwd->pw_name,pwd->pw_gid,pwd->pw_gid);
	}
}

int main(int argc,char **argv){
	int i = parse_arg(argc,argv,options,arraylen(options));
	ret = 0;
	if(i >= argc){
		print_info(flags & FLAG_REAL ? getuid() : geteuid());
	} else {
		for(;i < argc; i++){
			print_info(str2uid(argv[i]));
		}
	}

	return ret;
}
