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
DBG	:= -DNDEBUG

#######################
# common tool locations
#######################
#
CLANG= clang
GCC= gcc
RM= rm

# Common C compiler warnings to silence
#
# Example: CSILENCE= -Wno-some-thing -Wno-another-thing
#
# NOTE: If you add -Wno-stuff to CSILENCE, please update
#	CUNKNOWN in the next comment block.
#
# NOTE: Please don't add -Wno-unknown-warning-option to CSILENCE.
#
CSILENCE= -Wno-char-subscripts

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
CSILENCE+=
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
PROG= prog
OBJ= ${PROG}.o
TARGET= ${PROG}$E

# list any data files supplied with the submission
#
DATA=


#################
# build the entry
#################
#
all: data ${TARGET}
	@:

.PHONY: all data try clean clobber

# how to compile
#
${PROG}: ${PROG}.c
	${CC} ${CFLAGS} $< -o $@ ${LDFLAGS}

# alternative executable
#
alt: data ${ALT_TARGET}
	@${TRUE}

${PROG}.alt: ${PROG}.alt.c
	${CC} ${CFLAGS} $< -o $@ ${LDFLAGS}

# data files
#
data: ${DATA}
	@${TRUE}

# both all and alt
#
everything: all alt
	@${TRUE}

# one suggested way to run the program
#
try: ${PROG} ${DATA}
	./${PROG} ${PROG}.c

###############
# utility rules
###############
#
clean:
	${RM} -f ${PROG}

clobber: clean
	${RM} -f ${TARGET}
