#
# Anthony's Editor
#
# IOCCC 1991
#
# Winner Best Utility
#

# Override from the command-line, eg. make DBG='-O0 -g'
DBG	:= -DNDEBUG

CFLAGS	:= -Os -Wall ${DBG}

# Cygwin
#LIBS	:= -lcurses

# NetBSD
LIBS	:= -lcurses -ltermcap

.c$E :
	${CC} ${CFLAGS} -DMODE=0600 -o $* $< ${LIBS}

all: build

build: ae ae-c89 ae-alt prog prog-alt

clean:
	-rm -f ae ae-c89 ae-alt prog* *.core

strip: build
	strip ae ae-c89 ae-alt prog prog-alt
	ls -l

# NOTE: some systems may need:
#
#  old SYS V:
#	cc ${CFLAGS} -DMODE=0600 -DBUF=32767 -o ant ant.c -lcurses
#  ATARI ST:
#	cc -DMODE=0 -DBUF=32767 -o ant.ttp ant.c curses.a
#  PC class:
#	wcl -mc -DMODE=0 -DBUF=32767 ant.c curses.lib
#
# This older version may generate lots of warnings with newer compilers.
#
ae: ae.c
	${CC} ${CFLAGS} -DMODE=0600 -DBUF=32767 ae.c -o $* ${LIBS}

# Convert from K&R to ANSI C89 and resolve compiler warnings.
ae-c89: ae-c89.c

# See comments.
ae-alt: ae-alt.c

# Compress source.
prog.c: ae-c89.c transform.sed
	sed -f transform.sed ae-c89.c | sed -e '/^[ 	]*$$/d' >$@

prog-alt.c: ae-alt.c transform.sed
	sed -f transform.sed ae-alt.c | sed -e '/^[ 	]*$$/d' >$@

prog: prog.c

prog-alt: prog-alt.c
