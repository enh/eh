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

CPPFLAGS:= -DBUF=${BUF} -DMODE=${MODE}

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
	-rm -f ae$E ae-c89$E ae-alt$E prog* *.core *.i a.out a.txt

strip: build
	strip ae$E ae-c89$E ae-alt$E prog$E prog-alt$E
	ls -l

size: build
	iocccsize -v1 prog.c
	iocccsize -v1 prog-alt.c

#######################################################################
# Generated files.
#######################################################################

prog.c: ae-c89.c transform.sed
	sed -f transform.sed ae-c89.c | \
	sed -e'/) {$$/{ N;N;s/ {\(.[[:blank:]]*[^;]*;.\)[[:blank:]]*}$$/\1/; }' | \
	sed -e'/^[[:blank:]]*$$/d' >$@

prog-alt.c: ae-alt.c transform.sed
	sed -f transform.sed ae-alt.c | \
	sed -e'/) {$$/{ N;N;s/ {\(.[[:blank:]]*[^;]*;.\)[[:blank:]]*}$$/\1/; }' | \
	sed -e'/^[[:blank:]]*$$/d' >$@

#######################################################################
# Tests
#
# Use a classic common well defined terminal, like `vt100` or `ansi`,
# which should keep the number of Curses operations to a minimum and
# are a subset of `xterm`, `screen, or `tmux`.  It is possible to use
# simple `ansi-mini`, `adm3a`, or `vt52` terminal definitions, but
# they would cause Curses to perform more complex redraw sequences.
#
# The basic terminals, like `dumb`, `glasstty`, `vanilla`, `lpr` are
# insufficient to drive Curses.  Try `TERM=ansi-mini` for bare bones.
#
#######################################################################

PROG	?= ./ae-alt$E

TITLE	= printf "     %s: " $@
PASS	= printf "\r-OK-\r\n"
RESET	= rm a.txt a.out 2>/dev/null; true
EMPTY	= cmp test/empty.out -
A_OUT	= cmp test/$@.out a.out
A_TXT	= diff -u test/$@.txt a.txt

test-all: build
	${MAKE} PROG=./ae-alt TERM=ansi LINES=24 COLUMNS=80 tests
	${MAKE} PROG=./prog-alt TERM=ansi LINES=24 COLUMNS=80 tests
	${MAKE} PROG=./ae-c89 TERM=ansi LINES=24 COLUMNS=80 tests
	${MAKE} PROG=./prog TERM=ansi LINES=24 COLUMNS=80 tests

# While the terminal definition will probably contain definitions for
# the number of hardware `lines` and `columns`, we explicitly override
# them to be sure to ensure terminal emulator dimensions are ignored.

test: build
	-${MAKE} PROG=${PROG} TERM=ansi LINES=24 COLUMNS=80 tests

tests:	redraw0 empty0 empty1 flip0 \
	ins0 ins1 ins2 ins3 ins4 ins5 \
	del0 del1 del2 del3 del4 del5 \
	scroll0 scroll1 goto0 goto1 goto2 goto3 \
	pgdn0 pgdn1 pgdn2 pgup0 pgup1 \
	ere0 ere1 ere2 ere3 ere4 ere5 ere6 ere7 ere8 ere9 ere10

# An unknown command character redraws the screen.
redraw0:
	@${TITLE}
	@${RESET}
	@printf "\aQ" | ${PROG} >a.out
	@${A_OUT}
	@${PASS}

# Start without file argument and stop.
empty0:
	@${TITLE}
	@${RESET}
	@printf "Q" | ${PROG} | ${EMPTY}
	@${PASS}

# Start without file argument, write an empty buffer, and stop.  Creates a.txt.
empty1:
	@${TITLE}
	@${RESET}
	@printf "WQ" | ${PROG} | ${EMPTY}
	@diff -u test/empty.txt a.txt
	@${PASS}

flip0:
	@${TITLE}
	@${RESET}
	@printf "iHELLO world!\e[~~~~~~~~~~~~WQ" | ${PROG} >a.out
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Start without file argument, insert nothing, and stop.
ins0:
	@${TITLE}
	@${RESET}
	@printf "i\eQ" | ${PROG} | ${EMPTY}
	@printf "i\fQ" | ${PROG} | ${EMPTY}
	@${PASS}

# Insert newline terminated, write, and stop.
ins1:
	@${TITLE}
	@${RESET}
	@printf "iHello\n\tworld!\n\eWQ" | ${PROG} >a.out
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Insert not newline terminated, write, and stop.
#
# + Insert `i` and type some text, newline, more text, `ESC`.  TOP should
#   remain unchanged, newline pushes `^D` down a row.
ins2:
	@${TITLE}
	@${RESET}
	@printf "iHello\n\twoot!\eWQ" | ${PROG} >a.out
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + Word right `L`, insert `i`, type some text, then `ESC`.  Any text
#   right of cursor should shift as new text inserted.
ins3:
	@${TITLE}
	@${RESET}
	@printf "iHello world!\e[wiWOOT! \fWQ" | ${PROG} >a.out
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + `G`, insert `i` and type in a word, newline, next word, `ESC`.  TOP
#    should remain unchanged, newline pushes `^D` down a row.
ins4:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf "Giinsert at EOF\npush EOF down\neach newline\n\eWQ" | ${PROG} >a.out
	@${A_OUT}
	@${A_TXT}
	@${PASS}

ins5:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf "j]i WOOT!\eWQ" | ${PROG} >a.out
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete from empty buffer.
del0:
	@${TITLE}
	@${RESET}
	@printf "xQ" | ${PROG} | ${EMPTY}
	@${PASS}

# Insert nothing, delete from empty buffer.
del1:
	@${TITLE}
	@${RESET}
	@printf "i\exQ" | ${PROG} | ${EMPTY}
	@${PASS}

# Insert single character, delete from buffer.
del2:
	@${TITLE}
	@${RESET}
	@printf "iZ\ehxWQ" | ${PROG} >a.out
	@${A_OUT}
	@diff -u test/empty.txt a.txt
	@${PASS}

# Insert some characters, delete last from buffer.
del3:
	@${TITLE}
	@${RESET}
	@printf "i123Z\ehxWQ" | ${PROG} >a.out
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Insert some characters with newline, delete newline from buffer.
del4:
	@${TITLE}
	@${RESET}
	@printf "i123Z\n\ehxWQ" | ${PROG} >a.out
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + Delete `x` a character.  Text to the right of cursor shifts left one.
del5:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf "GkxhxWQ" | ${PROG} >a.out
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + Cursor down `j` from BOF to EOF.  Only cursor moves.
# + Cursor up `k` from EOF to BOF.  Only cursor moves.
scroll0:
	@${TITLE}
	@printf "jjjjjjjjjjjkkkkkkkkkkkQ" | ${PROG} test/short.txt >a.out
	@${A_OUT}
	@${PASS}

# + Cursor down `j` from BOF to BOP.  Only cursor moves.
# + Cursor down `j` twice from BOP.  Screen scrolls up two logical lines.
# + Cursor up `k` to TOP.  Only cursor moves.
# + Cursor up `k` twice from TOP.  Screen scrolls down two logical lines.
scroll1:
	@${TITLE}
	@printf "jjjjjjjjjjjjjjjjjjjjjjjjkkkkkkkkkkkkkkkkkkkkkkkkQ" | ${PROG} test/usage.md >a.out
	@${A_OUT}
	@${PASS}

# The goto line `G` always repositions the target line at the top of
# the page by design.  This can look a little odd on short less than
# a screen of text, since it does not simply reposition the cursor
# within the screen.  Or jumping to the EOF, since there is no context
# displayed (just cursor up and down a line).
#
# One can debate the merits of this design choice when compared to
# other editors that try to position the target line in the middle
# of the screen or highlight the cursor row or both.  While pretty
# and probably less shocking, positioning the target line at top
# makes for consistency display and easier testing.

# Goto line `<num>G` where `<num>` is on the screen.  Screen redraws
# with target line at TOP.
goto0:
	@${TITLE}
	@printf "7G15G2G1GQ" | ${PROG} test/usage.md >a.out
	@${A_OUT}
	@${PASS}

# Cursor at BOF, cursor down `j` a few lines on the screen, `1G`.
# Cursor moves to BOF.
goto1:
	@${TITLE}
	@printf "jjjjjjjjjjjjjjj1GQ" | ${PROG} test/usage.md >a.out
	@${A_OUT}
	@${PASS}

# Goto EOF `G`.  No terminating newline.
goto2:
	@${TITLE}
	@printf "GhkkQ" | ${PROG} test/usage-nonl.md >a.out
	@${A_OUT}
	@${PASS}

# Goto EOF `G`.
goto3:
	@${TITLE}
	@printf "GhkkQ" | ${PROG} test/usage.md >a.out
	@${A_OUT}
	@${PASS}

# Page down `J`.  Short file, no terminating newline.
pgdn0:
	@${TITLE}
	@printf "JQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_OUT}
	@${PASS}

# Page down `J`.  Short file, terminating newline.
pgdn1:
	@${TITLE}
	@printf "JQ" | ${PROG} test/short.txt >a.out
	@${A_OUT}
	@${PASS}

# Page down to EOF.
pgdn2:
	@${TITLE}
	@printf "JJJJQ" | ${PROG} test/usage.md >a.out
	@${A_OUT}
	@${PASS}

# + Page up `K` once.  Screen remains unchanged with cursor at TOP and BOF.
# + Page down `J` once.  Cursor moves to EOF.
# + Page up `K`.  Screen redraws its starting point with cursor at TOP and BOF.
pgup0:
	@${TITLE}
	@printf "KJKQ" | ${PROG} test/short.txt >a.out
	@${A_OUT}
	@${PASS}

# Page down and up a couple of pages.
pgup1:
	@${TITLE}
	@printf "JJKKQ" | ${PROG} test/usage.md >a.out
	@${A_OUT}
	@${PASS}

# Goto EOF and page up to BOF.
pgup2:
	@${TITLE}
	@printf "GKKKKQ" | ${PROG} test/usage.md >a.out
	@${A_OUT}
	@${PASS}

# Invalid ERE pattern.
ere0:
	@${TITLE}
	@printf "/)(\nQ" | ${PROG} test/short.txt >a.out
	@${A_OUT}
	@${PASS}

# Start of lines.
ere1:
	@${TITLE}
	@printf "/^\nnnnnnnnnnnQ" | ${PROG} test/short.txt >a.out
	@${A_OUT}
	@${PASS}

# Start of lines.
ere2:
	@${TITLE}
	@printf "/^\nnnnnnnnnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_OUT}
	@${PASS}

# End of lines.
ere3:
	@${TITLE}
	@printf "/$$\nnnnnnnnnnnQ" | ${PROG} test/short.txt >a.out
	@${A_OUT}
	@${PASS}

# End of lines.
ere4:
	@${TITLE}
	@printf "/$$\nnnnnnnnnnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_OUT}
	@${PASS}

# Last character of line.
ere5:
	@${TITLE}
	@printf "/.$$\nnnnnnnnnQ" | ${PROG} test/short.txt >a.out
	@${A_OUT}
	@${PASS}

# Last character of line.
ere6:
	@${TITLE}
	@printf "/.$$\nnnnnnnnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_OUT}
	@${PASS}

# Blank lines.
ere7:
	@${TITLE}
	@printf "/^$$\nnnQ" | ${PROG} test/short.txt >a.out
	@${A_OUT}
	@${PASS}

# Blank lines.
ere8:
	@${TITLE}
	@printf "/^$$\nnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_OUT}
	@${PASS}

# Plain text
ere9:
	@${TITLE}
	@printf "/line\nnnnQ" | ${PROG} test/short.txt >a.out
	@${A_OUT}
	@${PASS}

# Blank lines.
ere10:
	@${TITLE}
	@printf "/line\nnnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_OUT}
	@${PASS}
