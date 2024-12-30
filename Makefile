#
# Anthony's Editor - IOCCC 1991 - Winner Best Utility
#

.POSIX :

O := .o
E :=

.SUFFIXES :
.SUFFIXES : .c .h .i $O $E

BUF	:= 65536
MODE	:= 0600

# Override from the command-line, eg. make DBG='-O0 -g'
DBG	:= -DNDEBUG

# gcc option closest to K&R behaviour, since no option -std=K&R.
# clang supports K&R, but unclear how to enable that support.
CFLAGS	:= -Wno-implicit ${DBG}

CPPFLAGS:= -DMODE=${MODE} -DBUF=${BUF}

LIBS	:= -lcurses -ltermcap

.c.i:
	${CC} ${CFLAGS} ${CPPFLAGS} -E $*.c >$*.i

.c$E :
	${CC} ${CFLAGS} ${CPPFLAGS} -o $*$E $< ${LIBS}

all: ant$E

clean:
	-rm -f ant$E *.core *.i a.out

ant$E: ant.c
