### IOCCC
/#define EXT/d
/^#ifdef.*EXT/,/^#else.*EXT/d
/^#endif.*EXT/d

/adjmarks/d
/growgap/d
/#define MATCHES/d
s/MATCHES/1/
/^#define getsigch/d
s/(^|[^[:alnum:]_])getsigch/getch/g
### End IOCCC
