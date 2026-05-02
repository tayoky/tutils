#include <deflate.h>
#include <stdio.h>
#include <tutils.h>

#define FLAG_STDOUT 0x01
#define FLAG_DECOMPRESS 0x01
#define FLAG_FORCE 0x01
#define FLAG_KEEP 0x01

static int ret = 0;

static opt_t gzip_opts[] = {
	OPT('c', NULL, FLAG_STDOUT, "output to stdout"),
	OPT('d', NULL, FLAG_DECOMPRESS, "decompress files instead of compressing them"),
	OPT('f', NULL, FLAG_FORCE, "allow overwriting output files"),
	OPT('k', NULL, FLAG_KEEP, "keep input files, default is to remove them"),
};

CMD(gzip, "gzip [OPTIONS] [FILES...]\n"
"compress files in .gz\n", gzip_opts);


static int gzip_main(int argc, char **argv) {
	error("TODO");
	return 0;
}
