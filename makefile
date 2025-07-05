SRC = $(shell find src -name "*.c" -not -name "stdopt.c" | xargs -L 1 basename)

EXE = $(addprefix bin/,${SRC:.c=})

include config.mk

CFLAGS += -DHOST="$(HOST)" $(OPT)

all :  $(EXE)
bin/% : src/%.c src/stdopt.c
	@mkdir -p bin
	@echo "[compiling $(shell basename $@)]"
	@$(CC) $(CFLAGS) -o $@ $^
clean :
	rm -f $(EXE)
install : $(EXE)
	@mkdir -p $(PREFIX)/bin
	cp $(EXE) $(PREFIX)/bin
config.mk :
	$(error "run ./configure before running make")
