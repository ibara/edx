# edx Makefile

PROG =	edx
OBJS =	edx.o

CC ?=		cc
CFLAGS =	-O2 -pipe -I/usr/X11R6/include
LDFLAGS =	-L/usr/X11R6/lib

all: ${OBJS}
	${CC} ${LDFLAGS} -o edx ${OBJS} -lX11

clean:
	rm -f ${PROG} ${OBJS}
