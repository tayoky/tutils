#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <tutils.h>

CMD_NOPT(whoami, "whoami [OPTION]\n"
"print current username or uid\n");

static int whoami_main(int argc,char **argv){
	struct passwd *pwd = getpwuid(geteuid());
	if(pwd){
		puts(pwd->pw_name);
		return 0;
	} else {
		puts("unknow");
		return 1;
	}
}
