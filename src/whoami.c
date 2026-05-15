#include <pwd.h>
#include <stdio.h>
#include <tutils.h>
#include <unistd.h>

CMD_NOPT(whoami, "whoami [OPTION]\n"
				 "print current username or uid\n");

static int whoami_main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	struct passwd *pwd = getpwuid(geteuid());
	if (pwd) {
		puts(pwd->pw_name);
		return 0;
	} else {
		puts("unknow");
		return 1;
	}
}
