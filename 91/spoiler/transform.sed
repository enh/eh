#
#  Keywords & Phrases
#
/^#ifndef NDEBUG/,/#endif.*NDEBUG/d
/[[:<:]]error(/d
/[[:<:]]EPRINTF/d
/[[:<:]]INFO/d
/[[:<:]]INFO/d
/[[:<:]]DEBUG/d
/[[:<:]]DUMP/d

/#define FAST/d
/^#ifdef.*FAST/,/^#else.*FAST/d
/^#endif.*FAST/d

# Assertions
/^#include.*assert/d
/assert(/d

# Comment lines
/^\/\//d
s/\/\*.*\*\///

# Comment blocks
/\/\*/,/\*\//d

# Trailing comments
s/[[:blank:]]*\/\///

#
# Replace macros.
#

/^#define TAB/d
s/TABSTOP(\([^)]*\))/(8-(\1\&7))/g

/^#ifndef.*MAX_COLS/,/^#endif$/d
/^#define MAX_COLS/d
s/MAX_COLS/999/g


#
#  Types
#

s/const //g
s/static //g
s/ssize_t /long /g
s/(void) \([[:alpha:]]\)/\1/

# Since sizeof (int) <= sizeof (off_t) using a smaller type like
# `int` actually results in binary size of prog-alt < prog (just).
# Otherwise long, off_t, and ptrdiff_t have prog < prog-alt
#
s/off_t */int /g
s/ptrdiff_t */int /g
#s/off_t */long /g
#s/ptrdiff_t */long /g

#
# Constant expansion
#

s/NULL/0/g
s/'\\0'/0/g

#
#  Functions & Types
#

s/[[:<:]]ptr[[:>:]]/Z/g
s/[[:<:]]pos[[:>:]]/P/g
s/[[:<:]]movegap[[:>:]]/G/g
s/[[:<:]]prevline[[:>:]]/M/g
s/[[:<:]]nextline[[:>:]]/N/g
s/[[:<:]]adjust[[:>:]]/A/g
s/[[:<:]]left[[:>:]]/L/g
s/[[:<:]]down[[:>:]]/D/g
s/[[:<:]]up[[:>:]]/U/g
s/[[:<:]]right[[:>:]]/R/g
s/[[:<:]]wleft[[:>:]]/B/g
s/[[:<:]]pgdown[[:>:]]/J/g
s/[[:<:]]pgup[[:>:]]/K/g
s/[[:<:]]wright[[:>:]]/W/g
s/[[:<:]]lnbegin[[:>:]]/H/g
s/[[:<:]]lnend[[:>:]]/E/g
s/[[:<:]]lngoto[[:>:]]/V/g
s/[[:<:]]top[[:>:]]/T/g
s/[[:<:]]bottom[[:>:]]/S/g
s/[[:<:]]insert[[:>:]]/I/g
s/[[:<:]]delete[[:>:]]/X/g
s/[[:<:]]file[[:>:]]/F/
s/[[:<:]]redraw[[:>:]]/C/g
s/[[:<:]]quit[[:>:]]/Q/g
s/[[:<:]]noop[[:>:]]/O/g
s/[[:<:]]display[[:>:]]/Y/


#
#  Variables
#
#  a, i, j, m, n, *s, *t, are used for parameters and local variables.
#

s/argc/x/g
s/argv/y/g
s/[[:<:]]off\(set\)*[[:>:]]/n/g
s/[[:<:]]cur[[:>:]]/m/g
s/[[:<:]]col\(umn\)*[[:>:]]/a/g
s/[[:<:]]key[[:>:]]/k/g
s/[[:<:]]func[[:>:]]/w/g
s/[[:<:]]done[[:>:]]/d/g
s/[[:<:]]here[[:>:]]/o/g
s/[[:<:]]page[[:>:]]/u/g
s/[[:<:]]epage[[:>:]]/v/g
s/[[:<:]]eof[[:>:]]/w/g
s/[[:<:]]count[[:>:]]/z/g
s/[[:<:]]buf[[:>:]]/b/g
s/[[:<:]]ebuf[[:>:]]/c/g
s/[[:<:]]gap[[:>:]]/g/g
s/[[:<:]]egap[[:>:]]/h/g
s/[[:<:]]fp[[:>:]]/f/g
s/[[:<:]]filename[[:>:]]/l/g
s/[[:<:]]cur_row[[:>:]]/y/g
s/[[:<:]]cur_col[[:>:]]/x/g
s/[[:<:]]ch[[:>:]]/a/g

#
#  Expressions
#

#    (type *) 0		->   0
s/([a-z ]*\*) *0/0/g

#    if (0 == expr)	->  if (!expr)
s/0 == /!/g

#    if (expr != 0)	->  if (expr)
s/ != 0//g

#    if (0 != expr)	->  if (expr)
s/0 != //g

#    (v = func(expr)) == 0	->  !(v = func(expr))
s/\(([a-zA-Z0-9][a-zA-Z0-9]* = [^)]*))\) == 0/!\1/g

#    (expr) == 0	->  !(expr)
s/\(([^()]*)\) == 0/!\1/g

#    expr == 0	->  !expr
s/\([^()]*\) == 0/!\1/g

#    func(expr) == 0	->  !func(expr)
s/\([a-zA-Z0-9][a-zA-Z0-9]*([^)]*)\) == 0/!\1/g

#    array[0]	->  *array
s/\([^ 	]*[^[]\)\[0\]/*\1/g

#    &array[n]	->  array+n
#N#s/\&\([^[]*\)\[\([^]]*\)\]/\1+\2/g


#
# Handle if() late.
#

#R#s/if[[:blank:]]*(/Q /
#R#1i\
#R##define Q if(
#R#

