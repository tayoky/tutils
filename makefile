MAKEFLAGS += --no-builtin-rules

SRC = $(shell find src -name "*.c" -not -name "stdopt.c" -not -name "grid.c" | xargs -L 1 basename)

EXE = $(addprefix bin/,${SRC:.c=})

include config.mk

CFLAGS += -DHOST="$(HOST)" $(OPT)

all :  $(EXE)
bin/% : build/%.o build/stdopt.o build/grid.o
	@mkdir -p bin
	@echo "[linking $(shell basename $@)]"
	@$(CC) $(CFLAGS) -o $@ $^ ../tlibc/build/src/stdlib/qsort.o

build/%.o : src/%.c
	@echo "[compiling $^]"
	@mkdir -p $(shell dirname $@)
	@$(CC) $(CFLAGS) -o $@ -c $^

clean :
	rm -fr bin build

install : $(EXE)
	@mkdir -p $(PREFIX)/bin
	cp $(EXE) $(PREFIX)/bin

config.mk :
	$(error "run ./configure before running make")

.PHONY : all clean install bin/%
.PRECIOUS : build/%.o
