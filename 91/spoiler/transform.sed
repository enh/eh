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

/^#include.*limits/d
/^#pragma/d

/^#ifdef.*FAST/,/^#else/d
/^#endif.*FAST/d

/^#if.*BUF/,/^#endif/d

/^#if.*MODE/,/^#endif/d
s/MODE/0600/

/#define TOP_LINE/d
#/^#if.*TOP_LINE/,/^#endif/d
#s/TOP_LINE/0/
s/TOP_LINE/1/

/^#define ROWS/d
#s/ROWS/LINES/g
s/ROWS/LINES-1/g


/#define STANDOUT_EOF/d
/^#ifdef.*STANDOUT_EOF/,/^#else/d
/^#endif.*STANDOUT_EOF/d
#/standout()/d
#/standend()/d

# Assertions
/^#include.*assert/d
/assert(/d

# Comment lines
/^[[:blank:]]*\/\//d
/^[[:blank:]]*\/\*.*\*\//d
#s/^[[:blank:]]*\/\*.*\*\///

# Trailing comment.
s/[[:blank:]]*\/\/[[:blank:]]*$//

# Comment blocks
/^[[:blank:]]*\/\*/,/^[[:blank:]]*\*\//d

# Inline comment.
s/\/\*.*\*\///

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
s/^\([_[:alpha:]][_[:alnum:]]*\)(void)$/\1()/

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
s/[[:<:]]movegap[[:>:]]/V/g
s/[[:<:]]prevline[[:>:]]/M/g
s/[[:<:]]nextline[[:>:]]/N/g
s/[[:<:]]bol[[:>:]]/O/g
s/[[:<:]]col_or_eol[[:>:]]/A/g
s/[[:<:]]row_start[[:>:]]/_/g
s/[[:<:]]left[[:>:]]/L/g
s/[[:<:]]down[[:>:]]/D/g
s/[[:<:]]up[[:>:]]/U/g
s/[[:<:]]right[[:>:]]/R/g
s/[[:<:]]wleft[[:>:]]/B/g
s/[[:<:]]pgdown[[:>:]]/J/g
s/[[:<:]]pgup[[:>:]]/K/g
s/[[:<:]]pgtop[[:>:]]/J_/g
s/[[:<:]]pgbottom[[:>:]]/K_/g
s/[[:<:]]wright[[:>:]]/W/g
s/[[:<:]]lnbegin[[:>:]]/H/g
s/[[:<:]]lnend[[:>:]]/E/g
s/[[:<:]]lngoto[[:>:]]/G/g
s/[[:<:]]top[[:>:]]/T/g
s/[[:<:]]bottom[[:>:]]/T_/g
s/[[:<:]]insert[[:>:]]/I/g
s/[[:<:]]del\(ete\)*[[:>:]]/X/g
s/[[:<:]]flipcase[[:>:]]/R_/g
s/[[:<:]]save[[:>:]]/S/
s/[[:<:]]redraw[[:>:]]/C_/g
s/[[:<:]]quit[[:>:]]/Q/g
s/[[:<:]]display[[:>:]]/Y/
s/[[:<:]]search[[:>:]]/F/
s/[[:<:]]next[[:>:]]/F_/
s/[[:<:]]clr_to_eol[[:>:]]/C/



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
s/[[:<:]]maxcol[[:>:]]/n/g
s/[[:<:]]key[[:>:]]/k/g
s/[[:<:]]func[[:>:]]/k_/g
s/[[:<:]]done[[:>:]]/d/g
s/[[:<:]]here[[:>:]]/o/g
s/[[:<:]]page[[:>:]]/u/g
s/[[:<:]]epage[[:>:]]/v/g
s/[[:<:]]eof[[:>:]]/j/g
s/[[:<:]]mark[[:>:]]/i/g
s/[[:<:]]count[[:>:]]/z/g
s/[[:<:]]buf[[:>:]]/b/g
s/[[:<:]]ebuf[[:>:]]/c/g
s/[[:<:]]gap[[:>:]]/g/g
s/[[:<:]]egap[[:>:]]/h/g
#s/[[:<:]]fp[[:>:]]/f/g
s/[[:<:]]filename[[:>:]]/f/g
s/[[:<:]]cur_row[[:>:]]/y/g
s/[[:<:]]cur_col[[:>:]]/x/g
s/[[:<:]]ch[[:>:]]/a/g
s/[[:<:]]ere[[:>:]]/e/g
s/[[:<:]]ere_dollar_only[[:>:]]/w/g
s/[[:<:]]match_length[[:>:]]/l/g
s/[[:<:]]matches[[:>:]]/m/g

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
s/\(([^(),]*)\) == 0/!\1/g

#    expr == 0	->  !expr
s/\([^(),]*\) == 0/!\1/g

#    func(expr) == 0	->  !func(expr)
s/\([a-zA-Z0-9][a-zA-Z0-9]*([^)]*)\) == 0/!\1/g


#    array[0].mem	->  array->
s/\([^ 	]*[^[]\)\[0\]\./\1->/g


#    array[0]	->  *array
s/\([^ 	]*[^[]\)\[0\]\([^.]\)/*\1\2/g

#    &array[n]	->  array+n
#N#s/\&\([^[]*\)\[\([^]]*\)\]/\1+\2/g

#
# Split `} else`
#
s/^\([[:blank:]]*\)} else/\1}\
\1else/
