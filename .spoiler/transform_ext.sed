/#define EXT/d
/^#ifdef.*EXT/d
/^#else.*EXT/,/^#endif.*EXT/d
s/mblength([^[:alnum:]_]|$)/S\1/g
s/nextch([^[:alnum:]_]|$)/k\1/g
s/prevch([^[:alnum:]_]|$)/Ρ\1/g
s/(^|[^[:alnum:]_])func([^[:alnum:]_]|$)/\1Κ\2/g
s/(^|[^[:alnum:]_])writefile([^[:alnum:]_]|$)/\1Δ\2/
