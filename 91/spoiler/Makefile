#
# Anthony's Editor
#
# IOCCC 1991
#
# Winner Best Utility
#

CFLAGS	= -Os -Wall

# Cygwin
#LIBS	= -lcurses

# NetBSD
LIBS	= -lcurses -ltermcap

.c$E :
	${CC} ${CFLAGS} -DMODE=0600 -o $* $< ${LIBS}

all: build

build: ae ae-c89

clean:
	-rm -f ae ae-c89

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
