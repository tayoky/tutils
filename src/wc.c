#include <stdio.h>
#include <grid.h>
#include <stdlib.h>
#include <tutils.h>

#define FLAG_BYTES 0x01
#define FLAG_WORDS 0x02
#define FLAG_LINES 0x04

struct stats {
	size_t bytes;
	size_t words;
	size_t lines;
	const char *name;
};

static char **grid;
static size_t grid_index = 0;
static size_t lines_count = 0;
static struct stats total = {0, 0, 0, "total"};
static int ret = 0;

static opt_t opts[] = {
	OPT('c', "--bytes" ,FLAG_BYTES, "print bytes count"),
	OPT('w', "--words" ,FLAG_WORDS, "print words count"),
	OPT('l', "--lines" ,FLAG_LINES, "print newlines count"),
};

CMD(wc, "wc [OPTIONS] [FILES]\n"
"print newline word and byte count\n",
opts);

static void add_stat(struct stats *stat) {
	char buf[64];
	if (flags & FLAG_LINES) {
		sprintf(buf, "%zu", stat->lines);
		grid[grid_index++] = strdup(buf);
	}
	if (flags & FLAG_WORDS) {
		sprintf(buf, "%zu", stat->words);
		grid[grid_index++] = strdup(buf);
	}
	if (flags & FLAG_BYTES) {
		sprintf(buf, "%zu", stat->bytes);
		grid[grid_index++] = strdup(buf);
	}
	grid[grid_index++] = (char*)stat->name;
	lines_count++;
}

static int entries_per_line(void) {
	int count = 1;
	if (flags & FLAG_LINES) count++;
	if (flags & FLAG_WORDS) count++;
	if (flags & FLAG_BYTES) count++;
	return count;
}

static int is_blank(char c) {
	return c == '\0' || c == ' ' || c == '\n' || c == '\t';
}

static void wc(const char *name) {
	FILE *file;
	if (!strcmp(name, "-")) {
		file = stdin;
	} else {
		file = fopen(name, "r");
		if (!file) {
			perror(name);
			ret = 1;
			return;
		}
	}

	size_t size;
	char buf[4096];
	struct stats stat = {0, 0, 0, name};
	char prev = '\0';
	while ((size = fread(buf, 1, sizeof(buf), file))) {
		stat.bytes += size;
		for (size_t i=0; i<size; i++) {
			if (buf[i] == '\n') {
				stat.lines++;
			}
			if (is_blank(prev) && !is_blank(buf[i])) {
				stat.words++;
			}
			prev = buf[i];
		}
		if (size < sizeof(buf)) break;
	}

	if (file != stdin) fclose(file);

	add_stat(&stat);
	total.bytes += stat.bytes;
	total.words += stat.words;
	total.lines += stat.lines;
}

static int wc_main(int argc, char **argv) {
	if (!(flags & (FLAG_LINES | FLAG_WORDS | FLAG_BYTES))) {
		flags |= FLAG_LINES | FLAG_WORDS | FLAG_BYTES;
	}
	if (argc < 1) {
		grid = malloc(sizeof(char*) * entries_per_line());
		wc("-");
	} else {
		grid = malloc(sizeof(char*) * entries_per_line() * argc);
		for (int i=0; i<argc; i++) {
			wc(argv[i]);
		}
		if (argc > 1) {
			add_stat(&total);
		}
	}
	grid_print(grid, entries_per_line() * lines_count, entries_per_line());
	return ret;
}
