#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <tutils.h>

#define FLAG_COMPLEMENT 0x01
#define FLAG_DELETE     0x02
#define FLAG_SQUEEZE    0x04
static opt_t opts[] = {
	OPT('c', "--complement", FLAG_COMPLEMENT, "show the $PWD variable"),
	OPT('d', "--delete", FLAG_DELETE, "delete characters found in STRING1"),
	OPT('s', "--squeeze-repeats", FLAG_SQUEEZE, "replace instance of repeated characters by one"),
};

CMD(tr, "tr [OPTIONS] STRING1 [STRING2]\n"
"translate characters from stdin to stdout\n",
opts);

static int crtl_from_char(int c) {
	switch (c) {
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case '\0':
		return '\\';
	default:
		return c;
	}
}

static int find_index(const char *str, int c, int *offset) {
	if (offset) *offset = 0;
	for (int i=0; *str; i++) {
		if (*str == '\\') {
			str++;
			int test;
			if (*str) {
				test = crtl_from_char(*str);
				str++;
			} else {
				test = '\\';
			}
			if (c == test) return i;
			continue;
		}
		if (str[1] == '-' && str[2]) {
			if (str[0] > str[2]) {
				error("range '%c-%c' is in reverse order", str[0], str[2]);
				exit(1);
			}
			if (c >= str[0] && c <= str[2]) {
				if (offset) *offset = c - str[0];
				return i;
			}
			str += 3;
			continue;
		}
		if (strncmp(str, "[:", 2)) {
		}
		if (*str == c) {
			return i;
		}
		str++;
	}
	return -1;
}

static int for_index(const char *str, int index, int offset) {
	const char *ptr = str;
	while (*ptr) {
		if (!index) {
			if (ptr[1] == '-' && ptr[2]) {
				if (offset >= ptr[2] - ptr[0]) return ptr[2];
				return ptr[0] + offset;
			} else if (*ptr == '\\' && ptr[1]) {
				return crtl_from_char(ptr[1]);
			} else {
				return *ptr;
			}
		}
		if (ptr[1] == '-' && ptr[2]) {
			ptr += 3;
		} else if (*ptr == '\\' && ptr[1]) {
			ptr += 2;
		} else {
			ptr++;
		}
		index--;
	}
	return str[strlen(str)];
}

static int tr_main(int argc, char **argv) {
	if (argc < 1) {
		error("missing argument");
		return 1;
	} else if (argc > 2) {
		error("too many arguments");
		return 1;
	}

	if (flags & FLAG_DELETE) {
		if (argc == 2 && !(flags & FLAG_SQUEEZE)) {
			error("need only one argument when deleting");
			return 1;
		} else if (argc == 1 && (flags & FLAG_SQUEEZE)) {
			error("need two arguments when deleting and squeezing repeats");
			return 1;
		}
	} else if (argc == 1 && !(flags & FLAG_SQUEEZE)) {
		error("need two arguments when translating");
		return 1;
	}

	int c;
	int last_out = EOF;
	char *last = argv[argc-1];
	while ((c = getchar()) != EOF) {
		int offset;
		int index = find_index(argv[0], c, &offset);
		if (index >= 0) {
			if (flags & FLAG_DELETE) {
				continue;
			}
			if (argc == 2) c = for_index(argv[1], index, offset);
		}
		if (find_index(last, c, NULL) >= 0 && c == last_out && (flags & FLAG_SQUEEZE)) {
			continue;
		}
		putchar(c);
		last_out = c;
	}
	return 0;
}
