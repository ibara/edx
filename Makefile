all:
	${CC} ${CFLAGS} -DWORDSTAR ${LDFLAGS} edx.c -o edx ${LIBS}
