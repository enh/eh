### IOCCC
/#define EXT/d
/^#ifdef.*EXT/,/^#else.*EXT/d
/^#endif.*EXT/d

/#define WIDE/d
/^#ifdef.*WIDE/,/^#else.*WIDE/d
/^#endif.*WIDE/d

/#define nextch/d
/#define prevch/d

/adjmarks/d
/growgap/d
/setlocale/d
/#define MATCHES/d
s/MATCHES/1/
/^#define getsigch/d
s/(^|[^[:alnum:]_])getsigch/getch/g
s/(^|[^[:alnum:]_])func([^[:alnum:]_]|$)/\1k\2/g
s/(^|[^[:alnum:]_])writefile([^[:alnum:]_]|$)/\1S\2/
### End IOCCC
