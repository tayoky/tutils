MAKEFLAGS += --no-builtin-rules

SRC = $(shell find src -name "*.c")
OBJ = $(SRC:src/%.c=build/%.o)
EXE = tutils
VERSION = $(shell git describe --tags --always)
CMDS = [ \
	basename \
	cat \
	chmod \
	chown \
	clear \
	cp \
	dd \
	dirname \
	echo \
	false \
	find \
	gunzip \
	gzip \
	head \
	hex \
	id \
	ln \
	ls \
	mkdir \
	mv \
	pwd \
	readlink \
	realpath \
	rm \
	rmdir \
	seq \
	sleep \
	stat \
	test \
	touch \
	tr \
	true \
	unlink \
	wc \
	which \
	whoami \
	xargs\
	yes \
	zcat

include config.mk

CFLAGS += -DHOST="$(HOST)" -DVERSION='"$(VERSION)"' $(OPT) -Iinclude

all :  build/$(EXE)

build/$(EXE) : $(OBJ)
	@echo "[linking $(shell basename $@)]"
	@$(CC) $(CFLAGS) -o $@ $^

# TODO : bring back one exe per cmd mode
build/% : build/%.o build/main.o build/utils.o build/grid.o

build/%.o : src/%.c
	@echo "[compiling $^]"
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) -o $@ -c $^

clean :
	rm -fr bin build

install : all
	@mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp "build/$(EXE)" "$(DESTDIR)$(PREFIX)/bin"
	rm -f $(foreach CMD, $(CMDS), "$(DESTDIR)$(PREFIX)/bin/$(CMD)")
	$(foreach CMD, $(CMDS), ln -s "$(EXE)" "$(DESTDIR)$(PREFIX)/bin/$(CMD)" ;)

config.mk :
	$(error "run ./configure before running make")

.PHONY : all clean install
.PRECIOUS : build/%.o
