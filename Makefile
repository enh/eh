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
# Example: CSILENCE= -Wno-some-thing -Wno-another-thing
#
# NOTE: If you add -Wno-stuff to CSILENCE, please update
#	CUNKNOWN in the next comment block.
#
# NOTE: Please don't add -Wno-unknown-warning-option to CSILENCE.
#
# -Wno-char-subscripts			ctypes macros
# -Wno-incompatible-pointer-types	atexit(endwin)
#
CSILENCE= -Wno-char-subscripts -Wno-incompatible-pointer-types -Wno-unused-parameter

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
CINCLUDE=

# Other flags to pass to the C compiler
#
# Example: COTHER= -fno-math-errno
#
COTHER=

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

# C compiler to use
#
CC= cc

# Compiler add-ons or replacements for clang only
#
ifeq "$(findstring $(CLANG),${CC})" "$(CLANG)"
#
CSILENCE+= -Wno-strict-prototypes
#
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
	mkiocccentry -m ${MAKE} -Y -i.answers -ILICENSE.md -Ieh.tws -Iprog.ext.c .stage `pwd`

test:
	${MAKE} -f test/Makefile PROG=${PROG} $@

######################################
# optional include of 1337 hacker rulz
######################################

-include 1337.mk ../1337.mk ../../1337.mk
