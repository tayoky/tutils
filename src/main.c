#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <tutils.h>

// tutils's entry point

char *progname;
int flags;

void error(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "%s : ", progname);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
}

void version(void) {
	printf("tutils %s\n", VERSION);
	puts("see https://github.com/tayoky/tutils for last version");
	puts("Copyright (c) 2025-2026 Tayoky BSD 3-Clause");
	puts("This is free software: you are free to change and redistribute it.");
}

void help(command_t *command) {
	// first find size for left col
	size_t size = 0;
	for (size_t i = 0; i < command->options_count; i++) {
		size_t cur = 0;
		if (command->options[i].str) {
			cur += strlen(command->options[i].str) + 1;
		}
		if (command->options[i].c) {
			cur += 3;
		}
		if (size < cur) size = cur;
	}

	printf("usage : %s\n", command->usage);
	if (command->desc) {
		printf("%s\n", command->desc);
	}
	for (size_t i = 0; i < command->options_count; i++) {
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

void man(command_t *command) {
	// first find size for left col
	size_t size = 0;
	for (size_t i = 0; i < command->options_count; i++) {
		size_t cur = 0;
		if (command->options[i].str) {
			cur += strlen(command->options[i].str) + 1;
		}
		if (command->options[i].c) {
			cur += 2;
		}
		if (size < cur) size = cur;
	}

	char upper_name[256] = {0};
	for (size_t i=0; i<strlen(command->name); i++) {
		upper_name[i] = toupper(command->name[i]);
	}
	printf(".Dd 20 July 2026\n");
	printf(".Dt %s 1\n", upper_name);
	printf(".Os tutils %s\n", VERSION);
	printf(".Sh NAME\n");
	printf(".Nm %s\n", command->name);
	printf(".Nd do stuff\n");
	printf(".Sh SYNOPSIS\n");
	printf("%s\n", command->usage);
	printf(".Sh DESCRIPTION\n");
	printf("%s\n", command->desc);
	printf(".Sh OPTIONS\n");
	printf(".Bl -tag -width \"%*s\"\n", (int)size, "");
	for (size_t i = 0; i < command->options_count; i++) {
		printf(".It Fl %c", command->options[i].c);
		if (command->options[i].str) {
			printf(" Fl %s", command->options[i].str + 1);
		}
		putchar('\n');
		printf("%s\n", command->options[i].desc);
	}
	printf(".El\n");
	printf(".Sh EXIT STATUS\n");
	printf(".Ex -std\n");
	printf(".Sh AUTHOR\n");
	printf("Written by Tayoky\n");
}

static int command_cmp(const void *e1, const void *e2) {
	const command_t *cmd1 = *(command_t **)e1;
	const command_t *cmd2 = *(command_t **)e2;
	return strcmp(cmd1->name, cmd2->name);
}

static command_t *find_command(const char *name) {
	command_t search = {.name = name};
	command_t *search_ptr = &search;
	command_t **cmd = bsearch(&search_ptr, commands, commands_count, sizeof(command_t *), command_cmp);
	return cmd ? *cmd : NULL;
}

// return the new index
static int parse_arg(int argc, char **argv, int i, opt_t *opt) {
	if (i == argc - 1) {
		// no more elements
		error(_("expected argument after '%s'"), argv[i]);
		exit(1);
	}
	i++;
	switch (opt->arg_type) {
	case OPT_STR:;
		*(char **)opt->value = argv[i];
		break;
	case OPT_INT:;
		char *end;
		*(int *)opt->value = strtol(argv[i], &end, 0);
		if (end == argv[i] || *end) {
			error(_("invalid number to '%s' : '%s'"), argv[i - 1], argv[i]);
			exit(1);
		}
		break;
	case OPT_SIZE:;
		// TODO : support for suffix
		*(size_t *)opt->value = strtoul(argv[i], &end, 0);
		if (end == argv[i] || *end) {
			error(_("invalid number to '%s' : '%s'"), argv[i - 1], argv[i]);
			exit(1);
		}
		break;
	case OPT_MODE:;
		// TODO : parse in more format
		*(mode_t *)opt->value = strtoul(argv[i], &end, 8);
		if (end == argv[i] || *end) {
			error(_("invalid mode to '%s' : '%s'"), argv[i - 1], argv[i]);
			exit(1);
		}
		break;
	}
	return i;
}

// return the new index
static int parse_long_opt(int argc, char **argv, int i, command_t *cmd) {
	// special case for --, --help and --version
	if (!strcmp("--help", argv[i])) {
		help(cmd);
		exit(0);
	}
	if (!strcmp("--version", argv[i])) {
		version();
		exit(0);
	}
	if (!strcmp("--man", argv[i])) {
		man(cmd);
		exit(0);
	}
	for (size_t j = 0; j < cmd->options_count; j++) {
		if (!cmd->options[j].str || strcmp(argv[i], cmd->options[j].str)) continue;

		// we found a match
		flags |= cmd->options[j].flags;
		if (cmd->options[j].value) {
			i = parse_arg(argc, argv, i, &cmd->options[j]);
		}
		return i;
	}
	error(_("unknow option '%s' (see --help)"), argv[i]);
	exit(1);
}

// return the new index
static int parse_short_opt(int argc, char **argv, int i, command_t *cmd) {
	// used to skip over the next element
	// used for options that take an arg
	int skip_next = 0;

	for (size_t l = 1; argv[i][l]; l++) {
		for (size_t j = 0; j < cmd->options_count; j++) {
			if (cmd->options[j].c != argv[i][l]) continue;

			// we found a match
			flags |= cmd->options[j].flags;
			if (cmd->options[j].value) {
				parse_arg(argc, argv, i, &cmd->options[j]);
				skip_next = 1;
			}
			goto finish_short;
		}
		error(_("unknow option '-%c' (see --help)"), argv[i][l]);
		exit(1);
finish_short:
		continue;
	}
	if (skip_next) i++;
	return i;
}

static int parse_opts(int argc, char **argv, command_t *cmd) {
	flags = 0;
	int i;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') break;
		if (argv[i][1] == '-') {
			if (!strcmp("--", argv[i])) {
				i++;
				break;
			}
			i = parse_long_opt(argc, argv, i, cmd);
		} else {
			if (!cmd->options_count) {
				// let the command handle args itself
				break;
			}
			i = parse_short_opt(argc, argv, i, cmd);
		}
		continue;
	}

	return i;
}

int main(int argc, char **argv) {
	setup_locale();
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
		error(_("unknow command '%s'"), progname);
		return 1;
	}
	int i = 1;
	if (cmd->usage || cmd->options_count) i = parse_opts(argc, argv, cmd);
	return cmd->main(argc - i, &argv[i]);
}
