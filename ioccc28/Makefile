#!/usr/bin/env gmake
#
# eh - Edit Here - vi(1) the good parts version
#

.POSIX :

O := .o
E :=

.SUFFIXES :
.SUFFIXES : .c .h .i $O $E

# Buffer size 128KB
BUF	:= 131072

# File creation mode.
MODE	:= 0600

# Override from the command-line, eg. make DBG='-O0 -g'
# Assume ${DBG} is the tail of ${CFLAGS}
DBG	:= -DNDEBUG

BUILT	:= `date -u +'%a, %d %b %Y %H:%M:%SZ'`
COMMIT	:= `git describe --tags`

#############################
# shell used by this Makefile
#############################

SHELL	:= bash

#######################
# common tool locations
#######################

CC	:= cc
GCC	:= gcc
CLANG	:= clang
GINDENT	:= gindent
MV	:= mv
RM	:= rm
TRUE	:= true

#####################
# C compiler settings
#####################


# Common C compiler warnings to silence
#
# -Wno-char-subscripts			ctypes macros
# -Wno-incompatible-pointer-types	atexit(endwin)
# -Wno-unused-parameter			main(int argc, ...)
#
# -Wno-strict-prototypes		functions no arguments
# -Wno-missing-prototypes		clang functiosn without static
# -Wno-missing-variable-declarations	clang globals without static
#
CSILENCE := -Wno-char-subscripts -Wno-incompatible-pointer-types -Wno-unused-parameter \
	    -Wno-strict-prototypes -Wno-unused-value

# Attempt to silence unknown warnings
#
# If you add -Wno-stuff to CSILENCE, then please change CUNKNOWN to read:
#
#	CUNKNOWN= -Wno-unknown-warning-option
#
CUNKNOWN=

# Common C compiler warning flags
#
CWARN= -Wall -Wextra -pedantic ${CSILENCE} ${CUNKNOWN}

# The standard to compile against
#
# Your IOCCC submission is free to require older C standards, or
# even not specify a C standard at all.  We suggest trying
# for -std=gnu17, but that is not a requirement and you won't
# be penalized if you name CSTD empty or use another
# well known and reasonably widely implemented C standard.
#
CSTD= -std=gnu17

# Compiler bit architecture
#
# Example for 32-bitness: ARCH= -m32
# Example for 64-bitness: ARCH= -m64
#
# NOTE: Normally one should not specify a specific architecture.
#
ARCH=

# Defines that are needed to compile
#
# Example: -Dfoo -Dbar=baz
#
# Frack need extra #define to enable SUS standard strdup(), strndup().
CDEFINE= -DBUF=${BUF} -DMODE=${MODE} -D_XOPEN_SOURCE=700

# Include files that are needed to compile
#
# Example: CINCLUDE= -include stdio.h
#
# Some of the most commonly used includes like ctype.h, stdio.h,
# stdlib.h, and string.h should be ignored or counted as 2, eg.
#
#   #include <stdlib.h>		// count as 2
#
# Including them here doesn't "feel right", bit of cheat.
#
# Current for IOCCC28 there is no length limit for CINCLUDE, see Discord:
# https://discord.com/channels/1327029557718417438/1332198374249594920/1379483193358684170
#
CINCLUDE := -include curses.h -include ctype.h -include string.h \
	-include stdlib.h -include iso646.h -include regex.h \
	-include fcntl.h -include locale.h -include unistd.h

# Other flags to pass to the C compiler
#
# Example: COTHER= -fno-math-errno
#
COTHER=	-funsigned-char

# Optimization
#
#OPT= -O3 -g3

# Prefer building for small `very toight like a toiger`.
OPT= -Os

# Default flags for ANSI C compilation
#
CFLAGS= ${CSTD} ${CWARN} ${ARCH} ${CDEFINE} ${CINCLUDE} ${COTHER} ${OPT} ${DBG}

# Libraries needed to build
#
LDFLAGS=-lcurses

# Linux NCurses with wide character support.
#LDFLAGS	:= -lncursesw

# C compiler to use
#
CC= cc

# Compiler add-ons or replacements for clang only
#
ifeq "$(findstring $(CLANG),${CC})" "$(CLANG)"
#
CSILENCE+= -Wno-missing-prototypes \
	-Wno-missing-variable-declarations -Wno-extra-semi-stmt \
	-Wno-c99-compat -Wno-incompatible-function-pointer-types-strict \
	-Wno-unsafe-buffer-usage

# Behaves worse than Gcc -Wpedantic; "What if your mother-in-law was a programmer" annoying.
CWARN+= -Weverything
#
endif

# Specific add-ons or replacements for gcc only
#
ifeq "$(findstring $(GCC),${CC})" "$(GCC)"
#
CSILENCE+=
#
CWARN+=
#
endif

###########################################
# Special Makefile variables for this entry
###########################################

# what to build
#
PROG	= ./prog
OBJ	= ${PROG}$O
TARGET	= ${PROG}$E

#ALT_OBJ= ${PROG}.alt$O
#ALT_TARGET= ${PROG}.alt$E

# list any data files supplied with the submission
#
DATA=

.c.i:
	${CC} ${CFLAGS} -E $*.c >$*.i

#################
# build the entry
#################

all: data ${TARGET}

.PHONY: all data try clean clobber install test

# how to compile
#
${PROG}$E: ${PROG}.c
	${CC} ${CFLAGS} ${PROG}.c -o $@ ${LDFLAGS}

# alternative executable
#
alt: data ${ALT_TARGET}

${PROG}.alt$E: ${PROG}.alt.c
	${CC} ${CFLAGS} ${PROG}.alt.c -o $@ ${LDFLAGS}

# data files
#
data: ${DATA}

# both all and alt
#
everything: all alt

# one suggested way to run the program
#
try: ${PROG} ${DATA}
	./${PROG} ${PROG}.c

###############
# utility rules
###############

clean:
	-${RM} -f ${OBJ} *.i indent.c

clobber: clean
	-${RM} ${TARGET}

strip: build
	strip ${BUILD}
	ls -l ${BUILD}

size: prog.c
	-iocccsize -v1 $?

entry:
	-mkdir -p .stage
	-rm -rf .stage/*
	mkiocccentry -m ${MAKE} -Y -i.answers -Itest -Iprog.ext.c -IREADME.md .stage `pwd`

test:
	${MAKE} -f test/Makefile PROG=${PROG} $@

######################################
# optional include of 1337 hacker rulz
######################################

-include 1337.mk ../1337.mk ../../1337.mk
