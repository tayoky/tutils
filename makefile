SRC = $(shell find src -name "*.c" | xargs -L 1 basename)

EXE = ${SRC:.c=}

all :  ${EXE}
% : src/%.c
	${CC} ${CFLAGS} -o $@ $^
clean :
	rm ${EXE}

