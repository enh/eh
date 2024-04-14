#
# Edit Here
#
# The main development platform is currently NetBSD.
#

.POSIX :

.if ${.MAKE.OS} == "Cygwin"
O := .obj
E := .exe
.else
O := .o
E :=
.endif

.SUFFIXES :
.SUFFIXES : .c .h .i $O $E

BASE	?= eh
BUF	:= 65536
MODE	:= 0600

# Override from the command-line, eg. make DBG='-O0 -g'
DBG	:= -DNDEBUG

# Cygwin gcc complains about ctype macros char-subscripts, but not for
# ctype functions. eg. isspace(ch) vs (isspace)(ch)
GCC	:= -Wno-char-subscripts -fno-ident -flto
#-fno-asynchronous-unwind-tables -fno-unroll-loops -fdelete-null-pointer-checks

CFLAGS	:= -Os -Wall -std=c11 ${GCC} ${DBG}

# Frack gcc needs extra #define to enable SUS standard strdup(), strndup().
GCCCPP	:= -D_POSIX_C_SOURCE=200809L

CPPFLAGS:= -DBUF=${BUF} -DMODE=${MODE} ${GCCCPP}

LIBS	:= -lcurses

MANDIR	!= find /usr/local -type d -name man -maxdepth 2 | head -n1

.c.i:
	${CC} ${CFLAGS} ${CPPFLAGS} -E $*.c >$*.i

.c$E :
	${CC} ${CFLAGS} ${CPPFLAGS} -o $*$E $< ${LIBS}

#######################################################################
#
#######################################################################

all: build

build: ae$E eh$E prog$E

clean:
	-rm -f ae$E eh$E prog* *.core *.stackdump *.i a.out a.txt b.txt
	-rm -rf test/terminfo.cdb

strip: build
	strip ae$E eh$E prog$E
	ls -l

size: prog.c
	-iocccsize -v1 prog.c

install: eh.0 eh$E
	install -o 0 -g 0 -m 555 eh$E /usr/local/bin
	install -o 0 -g 0 -d ${MANDIR}/cat1
	install -o 0 -g 0 -p -m 444 eh.0 ${MANDIR}/cat1

#######################################################################
# Generated files.
#######################################################################

prog.c: ${BASE}.c transform.sed
	sed -E -f transform.sed ${BASE}.c | \
	sed -e'/) {$$/{ N;N;s/ {\(.[[:blank:]]*[^;]*;.\)[[:blank:]]*}$$/\1/; }' | \
	sed -e'/^[[:blank:]]*$$/d' >$@

#######################################################################
# Tests
#
# The basic terminals, like `dumb`, `glasstty`, `vanilla`, `lpr` are
# insufficient to drive NetBSD Curses.  Try `TERM=ansi-mini` for bare
# bones.
#
# For testing and debugging define a terminal where the defined escape
# sequences are readable text.  This avoids differences between terminal
# definitions across platforms.
#
# However, differences in Curses libraries, ie. NetBSD Curses vs Linux
# NCurses can reorder escape sequences to produce the same visual result.
# Similarly library upgrades might reorder escape sequences as seen between
# NetBSD 9.3 and 10.0.
#
#######################################################################

PROG	?= ./${BASE}$E
OS	?= ${.MAKE.OS}

# Set ASK=true to prompt to update a failed test.
ASK	?= false

# Set REBUILD=true to update all the failed tests.
REBUILD	?= false

TESTDIR	= test/${TERM}
RESET	= rm a.txt a.out *.core 2>/dev/null; true
TITLE	= printf "%s      %s: " ${PROG} $@ ; ${RESET}
PASS	= printf "\r%s -OK-\r\n" ${PROG}
PROMPT	= ( ${ASK} && echo -n "Replace y/N$$PS2" && read yn && test "$$yn" = 'y' )
SAVE	= ( ${REBUILD} || ${PROMPT} ) && cp a.out ${TESTDIR}/$@.out
A_CORE	= test ! -f *.core
A_OUT	= ( diff -u ${TESTDIR}/$@.out a.out || ${SAVE} )
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
	-${MAKE} PROG=${PROG} TERM=textterm LINES=24 COLUMNS=80 TERMINFO=$${PWD}/test/terminfo.cdb tests

tests:	tests_have_term_data \
	arg0 term0 empty0 empty1 redraw0 flip0 flip1 \
	ins0 ins1 ins2 ins3 ins4 ins5 \
	del0 del1 del2 del3 del4 del5 del6 \
	scroll0 scroll1 scroll2 \
	goto0 goto1 goto2 goto3 \
	pgdn0 pgdn1 pgdn2 pgdn3 pgdn4 pgup0 pgup1 pgup2 pgup3 \
	ere0 ere1 ere2 ere3 ere4 ere5 ere6 ere7 ere8 ere9 ere10 ere11 \
	mark0 mark1 mark2 mark3 mark4 \
	del7 del8 del9 del10 del11 del12 del13 \
	undo0 undo1 undo2 undo3 \
	paste0 paste1 paste2 paste3 \
	bang0 bang1 bang2 bang3 bang4 bang5 bang6 bang7

tests_have_term_data:
	@test -d ${TESTDIR}
	@echo TERM=${TERM}

# Allow missing filename argument.
arg0:
	@${TITLE}
	@printf "\aQ" | ${PROG} >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Dumb terminal.
term0:
	@${TITLE}
	@TERM=BOGUS ${PROG} a.txt 2>/dev/null || test $$? -eq 1
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
	@printf "Q" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Start, write an empty buffer, and stop.  Creates a.txt.
empty1:
	@${TITLE}
	@printf "W\nQ" | ${PROG} a.txt >a.out
	@diff -u test/empty.txt a.txt
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# This test does not work since backspace in redirected input is
# treated differently from a tty.  See NetBSD 10.0 lib/58151
empty2:
	@${TITLE}
	@printf 'W\b\b\b\b\bb.txt\nQ' | ${PROG} a.txt >a.out
	@test ! -f a.txt && diff -u test/empty.txt b.txt
	@${A_CORE}
	@${A_OUT}
	@${PASS}

flip0:
	@${TITLE}
	@printf "iHELLO world!\e|~~~~~~~~~~~~W\nQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

flip1:
	@${TITLE}
	@printf "iHello World!\e|12~W\nQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Start, insert nothing, and stop.
ins0:
	@${TITLE}
	@printf "i\eQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Insert newline terminated, write, and stop.
ins1:
	@${TITLE}
	@printf "iHello\n\tworld!\n\eW\nQ" | ${PROG} a.txt >a.out
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
	@printf "iHello\n\twoot!\eW\nQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + Word right `L`, insert `i`, type some text, then `ESC`.  Any text
#   right of cursor should shift as new text inserted.
ins3:
	@${TITLE}
	@printf "iHello world!\e|wiWOOT! \eW\nQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + `G`, insert `i` and type in a word, newline, next word, `ESC`.  TOP
#    should remain unchanged, newline pushes `^D` down a row.
ins4:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf "Giinsert at EOF\npush EOF down\neach newline\n\eW\nQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

ins5:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf 'j99|i WOOT!\eW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete from empty buffer.
del0:
	@${TITLE}
	@printf "xQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Insert nothing, delete from empty buffer.
del1:
	@${TITLE}
	@printf "i\exQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Insert single character, delete from buffer.
del2:
	@${TITLE}
	@printf "iZ\ehxW\nQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@diff -u test/empty.txt a.txt
	@${PASS}

# Insert some characters, delete last from buffer.
del3:
	@${TITLE}
	@printf "i123Z\ehxW\nQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Insert some characters with newline, delete newline from buffer.
del4:
	@${TITLE}
	@printf "i123Z\n\ehxW\nQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# + Delete `x` a character.  Text to the right of cursor shifts left one.
del5:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf "GkxhxW\nQ" | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

del6:
	@${TITLE}
	@printf "Hello\n\tworld!\n" >a.txt
	@printf "Gk6xW\nQ" | ${PROG} a.txt >a.out
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

# Set some marks, edit before the marks, marks remain unchanged.
mark2:
	@${TITLE}
	@printf '/butt\nmajmb/rude\nifuzz\e`a`bu`a`bQ' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

mark3:
	@${TITLE}
	@printf '/butt\nmajmb/rude\nd2w`a`bu`a`bQ' | ${PROG} test/short.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

mark4:
	@${TITLE}
	@printf '/butt\nmajmb/rude\ny2wPP`a`bu`a`bQ' | ${PROG} test/short.txt >a.out
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
	@printf '3wdjW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete whole line.
del9:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3w|djW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete to pattern and then next.
del10:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'wd/line\ndnW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete entire buffer.
del11:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'dGW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete back to mark.
del12:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'wma2jd`aW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Delete six words, ie. [count]d[count]motion
del13:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3w2d3wW\nQ' | ${PROG} a.txt >a.out
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
	@printf '3liWOOT!\euuW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Undo delete.
undo2:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3wd3wuuW\nQ' | ${PROG} a.txt >a.out
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
	@printf '3wdwj2w3PW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Paste at EOF.
paste3:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf '3wdwuGPkW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Command not found.
bang0:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'j!!WOOT\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Command fail.
bang1:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'j!!false\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Command success.
bang2:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'j!!true\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${PASS}

# Command read-only.
bang3:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'j!!ls test/term*\nW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Command save and delete region.
bang4:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'j!2jcat >b.txt\nW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@test $$(wc -c <b.txt) -eq 43
	@${PASS}

# Command save region.
bang5:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'j!2jtee b.txt\nW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@test $$(wc -c <b.txt) -eq 43
	@${PASS}

# Command format region fails.
bang6:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'j!2jfmt -w40\nW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}

# Command format region succeeds.
bang7:
	@${TITLE}
	@cp test/short.txt a.txt
	@printf 'j!2jfmt -g32 -w40\nW\nQ' | ${PROG} a.txt >a.out
	@${A_CORE}
	@${A_OUT}
	@${A_TXT}
	@${PASS}
