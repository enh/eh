#
# Anthony's Editor - IOCCC 1991 - Winner Best Utility
#

.POSIX :

O := .o
E :=

.SUFFIXES :
.SUFFIXES : .c .h .i $O $E

BASE	?= ae-c89
BUF	:= 131072
MODE	:= 0600

# Override from the command-line, eg. make DBG='-O0 -g'
DBG	:= -DNDEBUG

CFLAGS	:= -Os -Wall ${DBG}

CPPFLAGS:= -DBUF=${BUF} -DMODE=${MODE}

# Cygwin
LIBS	:= -lcurses

# NetBSD
#LIBS	:= -lcurses -ltermcap

.c.i:
	${CC} ${CFLAGS} ${CPPFLAGS} -E $*.c >$*.i

.c$E :
	${CC} ${CFLAGS} ${CPPFLAGS} -o $*$E $< ${LIBS}

#######################################################################
#
#######################################################################

all: build

build: ae$E ae-c89$E prog$E

clean:
	-rm -f ae$E ae-c89$E prog* *.core *.i a.out a.txt test/terminfo.cdb

strip: build
	strip ae$E ae-c89$E prog$E
	ls -l

size: prog.c
	-iocccsize -v1 prog.c

#######################################################################
# Generated files.
#######################################################################

prog.c: ${BASE}.c transform.sed
	sed -f transform.sed ${BASE}.c | \
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

PROG	?= ./${BASE}$E
REBUILD	?= false

TESTDIR	= test/${TERM}
RESET	= rm a.txt a.out *.core 2>/dev/null; true
TITLE	= printf "%s      %s: " ${PROG} $@ ; ${RESET}
PASS	= printf "\r%s -OK-\r\n" ${PROG}
SAVE	= ${REBUILD} && cp a.out ${TESTDIR}/$@.out
EMPTY	= cmp ${TESTDIR}/empty.out -
A_CORE	= test ! -f *.core
A_OUT	= ${SAVE} ; cmp ${TESTDIR}/$@.out a.out
A_TXT	= diff -u test/$@.txt a.txt

test-all: build
	${MAKE} PROG=./${BASE}$E test
	${MAKE} PROG=./prog$E test

# While the terminal definition will probably contain definitions for
# the number of hardware `lines` and `columns`, we explicitly override
# them to be sure to ensure terminal emulator dimensions are ignored.

test/terminfo.cdb : test/terminfo
	tic -x -o $@ test/terminfo

test: build test/terminfo.cdb
	-${MAKE} PROG=${PROG} TERM=textterm LINES=24 COLUMNS=80 TERMINFO=$${PWD}/test/terminfo tests
#	-${MAKE} PROG=${PROG} TERM=ansi LINES=24 COLUMNS=80 tests

tests:	tests_have_term_data \
	arg0 term0 empty0 empty1 redraw0 flip0 flip1 \
	ins0 ins1 ins2 ins3 ins4 ins5 \
	del0 del1 del2 del3 del4 del5 del6 \
	scroll0 scroll1 scroll2 \
	goto0 goto1 goto2 goto3 \
	pgdn0 pgdn1 pgdn2 pgdn3 pgdn4 pgup0 pgup1 pgup2 pgup3 \
	ere0 ere1 ere2 ere3 ere4 ere5 ere6 ere7 ere8 ere9 ere10 ere11 \
	mark0 mark1 del7 del8 del9 del10 del11 del12 del13 \
	undo0 undo1 undo2 undo3 \
	paste0 paste1 paste2 paste3

tests_have_term_data:
	@test -d ${TESTDIR}
	@echo TERM=${TERM}

# Missing filename argument.
arg0:
	@${TITLE}
	@${PROG} >/dev/null && test $$? -eq 0
	@${A_CORE}
	@${PASS}

# Dumb terminal.
term0:
	@${TITLE}
	@TERM=dumb ${PROG} a.txt ||  test $$? -eq 1
	@${A_CORE}
	@${PASS}

# An unknown command character redraws the screen.
redraw0:
	@${TITLE}
	@printf "\aQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Start and stop.
empty0:
	@${TITLE}
	@printf "Q" | ${PROG} a.txt | ${EMPTY}
	@${A_CORE}
	@${PASS}

# Start, write an empty buffer, and stop.  Creates a.txt.
empty1:
	@${TITLE}
	@printf "WQ" | ${PROG} a.txt | ${EMPTY}
	@diff -u test/empty.txt a.txt
	@${A_CORE}
	@${PASS}

flip0:
	@${TITLE}
	@printf "iHELLO world!\e|~~~~~~~~~~~~WQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

flip1:
	@${TITLE}
	@printf "iHello World!\e|12~WQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Start, insert nothing, and stop.
ins0:
	@${TITLE}
	@printf "i\eQ" | ${PROG} a.txt | ${EMPTY}
	@${A_CORE}
	@${PASS}
#	@printf "i\fQ" | ${PROG} a.txt | ${EMPTY}
#	@${A_CORE}

# Insert newline terminated, write, and stop.
ins1:
	@${TITLE}
	@printf "iHello\n\tworld!\n\eWQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Insert not newline terminated, write, and stop.
#
# + Insert `i` and type some text, newline, more text, `ESC`.  TOP should
#   remain unchanged, newline pushes `^D` down a row.
ins2:
	@${TITLE}
	@printf "iHello\n\twoot!\eWQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + Word right `L`, insert `i`, type some text, then `ESC`.  Any text
#   right of cursor should shift as new text inserted.
ins3:
	@${TITLE}
	@printf "iHello world!\e|wiWOOT! \eWQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + `G`, insert `i` and type in a word, newline, next word, `ESC`.  TOP
#    should remain unchanged, newline pushes `^D` down a row.
ins4:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf "Giinsert at EOF\npush EOF down\neach newline\n\eWQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

ins5:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf 'j99|i WOOT!\eWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete from empty buffer.
del0:
	@${TITLE}
	@printf "xQ" | ${PROG} a.txt | ${EMPTY}
	@${A_CORE}
	@${PASS}

# Insert nothing, delete from empty buffer.
del1:
	@${TITLE}
	@printf "i\exQ" | ${PROG} a.txt | ${EMPTY}
	@${A_CORE}
	@${PASS}

# Insert single character, delete from buffer.
del2:
	@${TITLE}
	@printf "iZ\ehxWQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@diff -u test/empty.txt a.txt
	@${PASS}

# Insert some characters, delete last from buffer.
del3:
	@${TITLE}
	@printf "i123Z\ehxWQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Insert some characters with newline, delete newline from buffer.
del4:
	@${TITLE}
	@printf "i123Z\n\ehxWQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + Delete `x` a character.  Text to the right of cursor shifts left one.
del5:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf "GkxhxWQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

del6:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf "Gk6xWQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + Cursor down `j` from BOF to EOF.  Only cursor moves.
# + Cursor up `k` from EOF to BOF.  Only cursor moves.
scroll0:
	@${TITLE}
	@printf "jjjjjjjjjjjkkkkkkkkkkkQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# + Cursor down `j` from BOF to BOP.  Only cursor moves.
# + Cursor down `j` twice from BOP.  Screen scrolls up two logical lines.
# + Cursor up `k` to TOP.  Only cursor moves.
# + Cursor up `k` twice from TOP.  Screen scrolls down two logical lines.
scroll1:
	@${TITLE}
	@printf "jjjjjjjjjjjjjjjjjjjjjjjjkkkkkkkkkkkkkkkkkkkkkkkkQ" | ${PROG} test/usage.md >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

scroll2:
	@${TITLE}
	@printf "24j24kQ" | ${PROG} test/usage.md >a.out
	@${A_CORE}
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
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Cursor at BOF, cursor down `j` a few lines on the screen, `1G`.
# Cursor moves to BOF.
goto1:
	@${TITLE}
	@printf "jjjjjjjjjjjjjjj1GQ" | ${PROG} test/usage.md >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Goto EOF `G`.  No terminating newline.
goto2:
	@${TITLE}
	@printf "GhkkQ" | ${PROG} test/usage-nonl.md >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Goto EOF `G`.
goto3:
	@${TITLE}
	@printf "GhkkQ" | ${PROG} test/usage.md >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Page down `J`.  Short file, no terminating newline.
pgdn0:
	@${TITLE}
	@printf "JQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Page down `J`.  Short file, terminating newline.
pgdn1:
	@${TITLE}
	@printf "JQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Page down to EOF.
pgdn2:
	@${TITLE}
	@printf "JJJJQ" | ${PROG} test/usage.md >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

pgdn3:
	@${TITLE}
	@printf "3JQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

pgdn4:
	@${TITLE}
	@printf "3JJQ" | ${PROG} test/usage.md >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# + Page up `K` once.  Screen remains unchanged with cursor at TOP and BOF.
# + Page down `J` once.  Cursor moves to EOF.
# + Page up `K`.  Screen redraws its starting point with cursor at TOP and BOF.
pgup0:
	@${TITLE}
	@printf "KJKQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Page down and up a couple of pages.
pgup1:
	@${TITLE}
	@printf "JJKKQ" | ${PROG} test/usage.md >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Goto EOF and page up to BOF.
pgup2:
	@${TITLE}
	@printf "GKKKKQ" | ${PROG} test/usage.md >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

pgup3:
	@${TITLE}
	@printf "G3KKQ" | ${PROG} test/usage.md >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Invalid ERE pattern.
ere0:
	@${TITLE}
	@printf "/)(\nQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Start of lines.
ere1:
	@${TITLE}
	@printf "/^\nnnnnnnnnnnQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Start of lines.
ere2:
	@${TITLE}
	@printf "/^\nnnnnnnnnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# End of lines.
ere3:
	@${TITLE}
	@printf "/$$\nnnnnnnnnnnQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# End of lines.
ere4:
	@${TITLE}
	@printf "/$$\nnnnnnnnnnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Last character of line.
ere5:
	@${TITLE}
	@printf "/.$$\nnnnnnnnnQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Last character of line.
ere6:
	@${TITLE}
	@printf "/.$$\nnnnnnnnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Blank lines.
ere7:
	@${TITLE}
	@printf "/^$$\nnnQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Blank lines.
ere8:
	@${TITLE}
	@printf "/^$$\nnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Plain text
ere9:
	@${TITLE}
	@printf "/line\nnnnQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Blank lines.
ere10:
	@${TITLE}
	@printf "/line\nnnnQ" | ${PROG} test/short-nonl.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# No match found.
ere11:
	@${TITLE}
	@printf "5w/WOOT\nQ" | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Unset set mark moves cursor to BOF.
mark0:
	@${TITLE}
	@printf '3w``Q' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@printf '3w`aQ' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@printf '3w`zQ' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Set a mark, move, goto mark, goto previous.
mark1:
	@${TITLE}
	@printf 'wm`Jb````Q' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@printf 'wmaJb`a``Q' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@printf 'wmzJb`z``Q' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# `dd` not supported.
del7:
	@${TITLE}
	@printf '3wddQ' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Delete from cursor to same column of next line.
del8:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3wdjWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete whole line.
del9:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3w|djWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete to pattern and then next.
del10:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'wd/line\ndnWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete entire buffer.
del11:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'dGWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete back to mark.
del12:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'wma2jd`aWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete six words, ie. [count]d[count]motion
del13:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3w2d3wWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Undo read file.
undo0:
	@${TITLE}
	@printf 'uuQ' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Undo insert.
undo1:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3liWOOT!\euuWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Undo delete.
undo2:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3wd3wuuWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete and undo entire buffer.
undo3:
	@${TITLE}
	@printf 'dGuQ' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Paste empty scrap.
paste0:
	@${TITLE}
	@printf 'PQ' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Delete buffer and paste back.
paste1:
	@${TITLE}
	@printf 'dGPQ' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Delete word, paste 3 times.
paste2:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3wdwj2w3PWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Paste at EOF.
paste3:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3wdwuGPkWQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}
