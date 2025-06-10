### IOCCC
/^#ifndef.*IOCCC/,/^#else.*IOCCC/d
/^#endif.*IOCCC/d

/#define WIDE/d
/^#ifdef.*WIDE/,/^#else.*WIDE/d
/^#endif.*WIDE/d

/#define nextch/d
/#define prevch/d
/#define setundo/d

/adjmarks/d
/growgap/d
/scrollup/d
#/setlocale/d
/#define MATCHES/d
s/MATCHES/1/
/^#define getsigch/d

/^#define .*_CMDS/d
s/MOTION_CMDS/14/g
s/(^|[^[:alnum:]_])getsigch/getch/g

### End IOCCC
