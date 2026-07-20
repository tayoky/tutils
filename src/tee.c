#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_SIGNAL
#include <signal.h>
#endif
#include <tutils.h>

#define FLAG_APPEND 0x1
#define FLAG_IGNORE 0x2

static int *fds;

static opt_t opts[] = {
	OPT('a', "--append", FLAG_APPEND, "append to files instead of overwriting them"),
	OPT('i', "--ignore-interrupts", FLAG_IGNORE, "ignore SIGINT"),
};

CMD(tee, "tee [OPTIONS] [FILES] ...",
	"Copy stdin to files and stdout.",
opts);

static int open_file(const char *path) {
	int open_flags = O_WRONLY | O_CREAT;
	if (flags & FLAG_APPEND) {
		open_flags |= O_APPEND;
	} else {
		open_flags |= O_TRUNC;
	}

	int fd = open(path, open_flags, 0666);
	if (fd < 0) {
		perror(path);
		return -1;
	}

	static int i = 0;
	fds[i++] = fd;
	return 0;
}

static int tee_main(int argc, char **argv) {
#ifdef HAVE_SIGNAL
	if (flags & FLAG_IGNORE) {
		signal(SIGINT, SIG_IGN);
	}
#endif
	
	fds = calloc(argc, sizeof(int));
	if (foreach_file(argv, open_file) < 0) {
		return 1;
	}

	ssize_t r;
	char buf[4096];
	int ret = 0;
	while ((r = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		for (int i=0; i<argc; i++) {
			if (fds[i] < 0) continue;
			ssize_t w = write(fds[i], buf, r);
			if (w < 0) {
				perror(argv[i]);
				fds[i] = -1;
				ret = 1;
				continue;
			}
		}
		if (write(STDOUT_FILENO, buf, r) < 0) {
			perror("stdout");
			ret = 1;
			break;
		}
	}

	for (int i=0; i<argc; i++) {
		if (fds[i] < 0) continue;
		close(fds[i]);
	}
	free(fds);
	return ret;
}
