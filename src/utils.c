#include <tutils.h>
#include <stdlib.h>
#include <pwd.h>

// random utils used commonly

uid_t str2uid(const char *str){
	char *end;
	uid_t ret = (uid_t)strtol(str,&end,10);
	if(end == str){
		struct passwd *pwd = getpwnam(str);
		if(!pwd){
			return -1;
		}
		ret = pwd->pw_uid;
	}
	return ret;
}
