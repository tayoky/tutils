#include <deflate.h>
#include <stdio.h>
#include <tutils.h>

#define FLAG_STDOUT 0x01
#define FLAG_DECOMPRESS 0x01
#define FLAG_FORCE 0x01
#define FLAG_KEEP 0x01

static opt_t gzip_opts[] = {
	OPT('c', NULL, FLAG_STDOUT, "output to stdout"),
	OPT('d', NULL, FLAG_DECOMPRESS, "decompress files instead of compressing them"),
	OPT('f', NULL, FLAG_FORCE, "allow overwriting output files"),
	OPT('k', NULL, FLAG_KEEP, "keep input files, default is to remove them"),
};

CMD(gzip, "gzip [OPTIONS] [FILES...]\n"
"compress files in .gz\n", gzip_opts);

static opt_t gunzip_opts[] = {
	OPT('c', NULL, FLAG_STDOUT, "output to stdout"),
	OPT('f', NULL, FLAG_FORCE, "allow reading from tty"),
	OPT('k', NULL, FLAG_KEEP, "keep input files, default is to remove them"),
};

CMD(gunzip, "gunzip [OPTIONS] [FILES...]\n"
"uncompress files in .gz\n", gunzip_opts);

static opt_t zcat_opts[] = {
	OPT('f', NULL, FLAG_FORCE, "allow reading from tty"),
};

CMD(zcat, "zcat [OPTIONS] [FILES...]\n"
"uncompress files in .gz to stdout\n", zcat_opts);

static int gzip(const char *path, FILE *in) {
	if (flags & FLAG_DECOMPRESS) {
		FILE *out;
		if ((flags & FLAG_STDOUT) || in == stdin) {
			out = stdout;
		} else {
			if (strlen(path) < 3 || strcmp(path + strlen(path) - 3, ".gz")) {
				error("%s : no .gz", path);
				return -1;
			}
			char out_path[strlen(path)+1];
			strcpy(out_path, path);
			out_path[strlen(out_path)-3] = '\0';
			// TODO : overwrite check
			out = fopen(out_path, "w");
			if (!out) {
				perror(out_path);
				return -1;
			}
		}
		int ret = inflate(in, out);
		if (out != stdin) {
			fclose(out);
			if (ret == 0 && !(flags & FLAG_KEEP)) {
				unlink(path);
			}
		}
		return ret;
	}
	error("TODO : compress files");
	return -1;
}

static int gzip_main(int argc, char **argv) {
	(void)argc;
	return -foreach_file_open(argv, gzip);
}

static int gunzip_main(int argc, char **argv) {
	// gunzip is just gzip -d
	flags |= FLAG_DECOMPRESS;
	return gzip_main(argc, argv);
}

static int zcat_main(int argc, char **argv) {
	// zcat is just gzip -dc
	flags |= FLAG_DECOMPRESS | FLAG_STDOUT;
	return gzip_main(argc, argv);
}
