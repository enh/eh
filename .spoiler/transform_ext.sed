/#define EXT/d
/^#ifdef.*EXT/d
/^#else.*EXT/,/^#endif.*EXT/d
s/(^|[^[:alnum:]_])writefile([^[:alnum:]_]|$)/\1Δ\2/
