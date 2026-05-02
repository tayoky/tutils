#include <tutils.h>
#include <stdlib.h>
#include <pwd.h>

// random utils used commonly

uid_t str2uid(const char *str){
	char *end;
	uid_t ret = (uid_t)strtol(str,&end,10);
	if(end == str || *end){
		struct passwd *pwd = getpwnam(str);
		if(!pwd){
			return -1;
		}
		ret = pwd->pw_uid;
	}
	return ret;
}

int foreach_file(char **argv, int (*callback)(const char *path)) {
	if (!*argv) {
		return callback("-");
	}
	int ret = 0;
	while (*argv) {
		if (callback(*argv) < 0) ret = -1;
		argv++;
	}
	return ret;
}

static FILE *file_open_read(const char *path) {
	if (!strcmp(path, "-")) {
		return stdin;
	}
	return fopen(path, "r");
}

int foreach_file_open(char **argv, int (*callback)(const char *path, FILE *file)) {
	if (!*argv) {
		return callback("-", stdin);
	}
	int ret = 0;
	while (*argv) {
		const char *path = *argv;
		argv++;
		FILE *file = file_open_read(path);
		if (!file) {
			perror(path);
			ret = -1;
			continue;
		}
		if (callback(path, file) < 0) ret = -1;
		if (file != stdin) fclose(file);
	}
	return ret;
}
