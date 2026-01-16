#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tutils.h>

// tutils's entry point

char *progname;
int flags;

void error(const char *fmt,...){
	va_list args;
	va_start(args, fmt);
	fprintf(stderr,"%s : ", progname);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
}

void version(void) {
	printf("tutils 0.6.0\n");
	printf("see https://github.com/tayoky/tutils for last version\n");
	printf("wrote by tayoky\n");
}

void help(command_t *command) {
	// first find size for left col
	int size = 0;
	for (int i=0; i<command->options_count; i++) {
		size_t cur = 0;
		if (command->options[i].str) {
			cur += strlen(command->options[i].str) + 1;
		}
		if (command->options[i].c) {
			cur += 3;
		}
		if (size < cur) size = cur;
	}

	printf("usage : %s", command->usage);
	for (int i=0; i<command->options_count; i++) {
		size_t cur = 0;
		if (command->options[i].str) {
			cur += strlen(command->options[i].str) + 1;
		}
		if (command->options[i].c) {
			cur += 3;
		}
		if (command->options[i].c) {
			printf("-%c ", command->options[i].c);
		}
		if (command->options[i].str) {
			printf("%s ", command->options[i].str);
		}

		// align
		while (cur < size) {
			putchar(' ');
			cur++;
		}
		printf(": %s\n", command->options[i].desc);
	}
}

static int command_cmp(const void *e1, const void *e2) {
	const command_t *cmd1 = *(command_t**)e1;
	const command_t *cmd2 = *(command_t**)e2;
	return strcmp(cmd1->name, cmd2->name);
}

static command_t *find_command(const char *name) {
	command_t search = {.name = name};
	command_t *search_ptr = &search;
	command_t **cmd = bsearch(&search_ptr, commands, commands_count, sizeof(command_t*), command_cmp);
	return cmd ? *cmd : NULL;
}

int parse_arg(int argc, char **argv, command_t *cmd){
	flags = 0;
	int i;
	for (i=1; i<argc;i++) {
		if (argv[i][0] != '-') break;
		if (argv[i][1] == '-') {
			//it's a long option
			//special case for --, --help and --version
			if (!strcmp("--", argv[i])) {
				i++;
				break;
			}
			if (!strcmp("--help",argv[i])) {
				help(cmd);
				exit(0);
			}
			if (!strcmp("--version",argv[i])) {
				version();
				exit(0);
			}
			for (int j=0; j<cmd->options_count; j++) {
				if (cmd->options[j].str && !strcmp(argv[i],cmd->options[j].str)) {
					flags |= cmd->options[j].flags;
					if (cmd->options[j].value) {
						if (i == argc-1) {
							error("expected argument after '%s'",argv[i]);
							exit(1);
						}
						i++;
						*cmd->options[j].value = argv[i];
					}
					goto finish;
				}

			}
			error("unknow option '%s' (see --help)",argv[i]);
			exit(1);
		} else {
			//it's a short options
			int skip_next = 0;
			for (int l=1; argv[i][l]; l++) {
				for (int j=0; j<cmd->options_count; j++) {
					if (cmd->options[j].c == argv[i][l]) {
						flags |= cmd->options[j].flags;
						if(cmd->options[j].value) {
							if (i == argc-1) {
								error("expected argument after '%c'",argv[i][l]);
								exit(1);
							}
							*cmd->options[j].value = argv[i+1];
							skip_next = 1;
						}
					
	goto finish_short;
					}

				}
				error("unknow option '-%c' (see --help)",argv[i][l]);
				exit(1);
finish_short:
				continue;
			}
			if (skip_next) i++;
		}
finish:
		continue;
	}

	return i;
}

int main(int argc, char **argv) {
	if (argc >= 1) {
		progname = strrchr(argv[0], '/');
		if (progname) {
			progname++;
		} else {
			progname = argv[0];
		}
	} else {
		progname = "tutils";
	}

	if (!strcmp(progname, "tutils") && argc >= 2 && argv[1][0] != '-') {
		progname = argv[1];
		argv = &argv[1];
		argc--;
	}

	command_t *cmd = find_command(progname);
	if (!cmd) {
		error("unknow command '%s'", progname);
		return 1;
	}
	int i=1;
	if (cmd->usage || cmd->options_count) i=parse_arg(argc, argv, cmd);
	return cmd->main(argc - i, &argv[i]);
}
