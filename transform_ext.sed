/#define EXT/d
/^#ifdef.*EXT/d
/^#else.*EXT/,/^#endif.*EXT/d
/^#define .*_CMDS/d
s/MOTION_CMDS/18/g
