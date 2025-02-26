SRC = $(shell find src -name "*.c" | xargs -L 1 basename)

EXE = ${SRC:.c=}
SYSROOT = 

all :  ${EXE}
% : src/%.c
	${CC} ${CFLAGS} -o $@ $^
clean :
	rm ${EXE}
install : all
	cp ${EXE} ${SYSROOT}/bin
