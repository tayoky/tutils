SRC = $(shell find src -name "*.c" | xargs -L 1 basename)

EXE = ${SRC:.c=}

include config.mk

CFLAGS += -DHOST="${HOST}"

all :  ${EXE}
% : src/%.c
	${CC} ${CFLAGS} -o $@ $^
clean :
	rm -f ${EXE}
install : ${EXE}
	cp ${EXE} ${PREFIX}/bin
config.mk :
	$(error "run ./configure before running make")
