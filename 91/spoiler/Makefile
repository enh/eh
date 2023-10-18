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

CFLAGS	:= -Os -Wall ${DBG}

CPPFLAGS:= -DMODE=${MODE}

# Cygwin
#LIBS	:= -lcurses

# NetBSD
LIBS	:= -lcurses -ltermcap

.c.i:
	${CC} ${CFLAGS} ${CPPFLAGS} -E $*.c >$*.i

.c$E :
	${CC} ${CFLAGS} ${CPPFLAGS} -o $*$E $< ${LIBS}

#######################################################################
#
#######################################################################

all: build

build: ae$E ae-c89$E ae-alt$E prog$E prog-alt$E

clean:
	-rm -f ae ae-c89 ae-alt prog* *.core *.i a.out

strip: build
	strip ae ae-c89 ae-alt prog prog-alt
	ls -l

# Original IOCCC 1991 version 1.0.0.
ae$E: ae.c
	${CC} ${CFLAGS} ${CPPFLAGS} -DBUF=${BUF} ae.c -o $*$E ${LIBS}

#######################################################################
# Generated files.
#######################################################################

prog.c: ae-c89.c transform.sed
	sed -f transform.sed ae-c89.c | sed -e '/^[ 	]*$$/d' >$@

prog-alt.c: ae-alt.c transform.sed
	sed -f transform.sed ae-alt.c | sed -e '/^[ 	]*$$/d' >$@

prog$E: prog.c

prog-alt$E: prog-alt.c
