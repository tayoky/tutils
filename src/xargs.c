#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <tutils.h>

#define FLAG_MAX_LINES  0x01
#define FLAG_MAX_ARGS   0x02
#define FLAG_PROMPT     0x04
#define FLAG_TRACE      0x08

static char *eof_string = "";
static int max_lines = 0;
static int max_args  = 0;
static int used_lines = 0;
static int used_args  = 0;
static int original_argc = 0;
static char **original_argv = 0;
static int ret = 0;

static opt_t opts[] = {
	OPTSTR('E', "--eof", 0, &eof_string, "set logical eof string to interpret as EOF"),
	OPTINT('L', "--max-lines", FLAG_MAX_LINES, &max_lines, "use a maximum of N non empty input lines per command"),
	OPTINT('n', "--max-args", FLAG_MAX_ARGS, &max_args, "use a maximum of N args per command"),
	OPT('p', "--interactive", FLAG_PROMPT | FLAG_TRACE, "ask before executing each command"),
	OPT('t', "--trace", FLAG_TRACE, "print each command before executing it"),
};

CMD(xargs, "xargs [OPTIONS...] [COMMAND [ARGUMENTS]]\n"
"parse args and execute commands\n", opts);

static void print_cmd(char **argv) {
	for (char **cur=argv; *cur; cur++) {
		if (cur[1]) {
			fprintf(stderr ,"%s ", *cur);
		} else {
			fprintf(stderr, "%s", *cur);
		}
	}
}

static void execute_cmd(char **argv) {
	if (flags & FLAG_PROMPT) {
		print_cmd(argv);
		fputc('\n', stderr);
	} else if (flags & FLAG_TRACE) {
		print_cmd(argv);
		fputc('\n', stderr);
	}

	fflush(NULL);
	pid_t child = fork();
	if (!child) {
		execvp(argv[0], argv);
		perror(argv[0]);
		exit(127);
	}
	if (child < 0) {
		perror("fork");
		exit(126);
	}
	int status;
	if (waitpid(child, &status, 0) < 0) {
		perror("waitpid");
		exit(126);
	}
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 127) {
			exit(127);
		} else if (WEXITSTATUS(status) == 0) {
			return;
		}

	}
	ret = 123;
	return;
}

static char **setup_args(void) {
	char **new_argv = malloc(original_argc * sizeof(char*));
	for (int i=0; i<original_argc; i++) {
		new_argv[i] = strdup(original_argv[i]);
	}
	return new_argv;
}

static void free_args(int argc, char **argv) {
	for (int i=0; i<argc; i++) {
		free(argv[i]);
	}
	free(argv);
}

static void add_arg(int *argc, char ***argv, const char *new_arg) {
	*argv = realloc(*argv, (*argc + 2) * sizeof(char*));
	(*argv)[*argc] = strdup(new_arg);
	(*argc)++;
	(*argv)[*argc] = NULL;

	used_args++;
	if ((flags & FLAG_MAX_ARGS) && used_args >= max_args) {
		execute_cmd(*argv);
		free_args(*argc, *argv);
		*argc = original_argc;
		*argv = setup_args();
		used_args = 0;
		used_lines = 0;
	}

}

static int xargs_main(int argc, char **argv) {
	if (argc < 1) {
		// by default execute echo
		argc = 1;
		argv = (char*[]){
			"echo",
			NULL,
		};
	}
	original_argc = argc;
	original_argv = argv;

	FILE *file = stdin;
	char buf[LINE_MAX];
	char arg[LINE_MAX];
	int cur_argc = argc;
	char **cur_argv = setup_args();

	while (fgets(buf, sizeof(buf), file)) {
		// ignore empty lines
		char *src = buf;
		while (isblank(*src)) src++;
		if (!*src) continue;

		used_lines++;
		if ((flags & FLAG_MAX_LINES) && used_lines >= max_lines) {
			error("TODO : run cmd");
			used_lines = 0;
			used_args  = 0;
		}

		int in_quote = 0;
		char *dest = arg;
		int prev_is_space = 1;
		while (*src) {
			if (in_quote == '\'') {
				prev_is_space = 0;
				if (*src == '\'') {
					in_quote = 0;
				} else {
					*(dest++) = *src;
				}
				src++;
				continue;
			}

			switch (*src) {
			case '"':
				in_quote = '"' - in_quote;
				prev_is_space = 0;
				break;
			case '\'':
				if (in_quote) goto default_case;
				in_quote = '\'';
				break;
			case '\\':
				if (!src[1]) break;
				src++;
				goto default_case;
			case ' ':
			case '\t':
			case '\n':
				if (in_quote) goto default_case;
				if (prev_is_space) break;
				prev_is_space = *src;
				*dest = '\0';
				add_arg(&cur_argc, &cur_argv, arg);
				dest = arg;
				break;
			default:
default_case:
				prev_is_space = 0;
				*(dest++) = *src;
				break;
			}
			src++;
		}
		if (in_quote) {
			error("non matching '%c'", in_quote);
			return 1;
		}
		
		if (!prev_is_space) {
			*dest = '\0';
			add_arg(&cur_argc, &cur_argv, arg);
		}
		if (prev_is_space != '\n') {
			// we didn't finish with newline
			// so EOF
			break;
		}
	}

	if (used_args > 0) {
		execute_cmd(cur_argv);
	}
	free_args(cur_argc, cur_argv);

	return ret;
}
