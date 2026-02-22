MAKEFLAGS += --no-builtin-rules

SRC = $(shell find src -name "*.c")
OBJ = $(SRC:src/%.c=build/%.o)
EXE = tutils
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
	head \
	hex \
	id \
	ln \
	ls \
	mkdir \
	mv \
	pwd \
	readlink \
	rm \
	rmdir \
	seq \
	sleep \
	stat \
	test \
	touch \
	true \
	unlink \
	whoami \
	wc \
	xargs\
	yes

include config.mk

CFLAGS += -DHOST="$(HOST)" $(OPT) -Iinclude

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
	@mkdir -p "$(PREFIX)/bin"
	cp "build/$(EXE)" "$(PREFIX)/bin"
	$(foreach CMD, $(CMDS), rm -f "$(PREFIX)/bin/$(CMD)"; ln -s "$(EXE)" "$(PREFIX)/bin/$(CMD)" ;)

config.mk :
	$(error "run ./configure before running make")

.PHONY : all clean install bin/%
.PRECIOUS : build/%.o
