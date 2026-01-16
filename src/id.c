#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <tutils.h>

#define FLAG_GROUPS 0x01
#define FLAG_GROUP  0x02
#define FLAG_USER   0x04
#define FLAG_REAL   0x08
#define FLAG_NAME   0x10

static int ret;

static opt_t opts[] = {
	OPT('u',"--user"  ,FLAG_USER  ,"print only effective user id"),
	OPT('g',"--group" ,FLAG_GROUP ,"print only effective group id"),
	OPT('G',"--groups",FLAG_GROUPS,"print only real, effective and supplementary groups id"),
	OPT('r',"--real"  ,FLAG_REAL  ,"print real id instead of effective"),
	OPT('n',"--name"  ,FLAG_NAME  ,"print the name (string) instead of an id (integer)"),
};

CMD(id,"id [OPTIONS] [USER]...\n"
"print user identity\n",
opts);

//TODO : support for suplementary groups + groups name
static void print_info(uid_t uid){
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
			printf(FUID"\n",pwd->pw_uid);
		}
	} else if(flags & FLAG_GROUP){
		printf(FGID"\n",pwd->pw_gid);
	} else if(flags & FLAG_GROUPS){
		printf("unsupported\n");
	} else {
		printf("uid="FUID"(%s) gid="FGID"("FGID")\n",pwd->pw_uid,pwd->pw_name,pwd->pw_gid,pwd->pw_gid);
	}
}

static int id_main(int argc, char **argv){
	ret = 0;
	if(argc < 1){
		print_info(flags & FLAG_REAL ? getuid() : geteuid());
	} else {
		for(int i=0;i < argc; i++){
			print_info(str2uid(argv[i]));
		}
	}

	return ret;
}
