48c574c3-3916-46c6-a5e1-b7a6bb760ef8 Slut 6
===========================================

### Building

Simply type `make` to build.  There are two macros that can be customised:

* `BUF` to set the buffer size; default 128KB.

* `MODE` to set the file creation mode; default 0600.


### What's Interesting.

* This is an update with bug fixes of a previous IOCCC winner, with permission of the author.

* This entry requires Curses.  Some Linux might require NCurses built with wide character support, `-lncursesw`.

* The source is UTF-8.  When are `é` `ë` `ê` `è` not same as `e`?

* The documentation is found in [prog.md](./prog.md).

* `prog` can view its own source.

* A regression test suite is available on demand.  The test suite requires `tic(1)` to build the specialised test terminal entry.

* Original unobfuscated more fully featured source available (search/replace, pipe-filter selection, markers, insert file, edit buffer grows, literal character input, multi undo/redo).

* The `iocccsize(1)` tool only counts bytes, not characters, ie. UTF-8 multibyte characters such that some characters cost 2, 3, or 4 bytes, thus penalising UTF-8 entries.

   > UTF-8 demands equal rights and respect as real characters!  The dominance of slim ASCII must end!  We are the UTF-8 People's Front for Freedom, Equality, and Brotherhood (oh that's been used already) - We are the UTF-8 Countless Tourist Defence Farce for One Voice, One World, One Character Set.


### Updates

* Refactor page framing to address original issue of scrolling and paging up/down when the file contains long lines.

* CTRL+C will exit insert mode as with `vi(1)`; CTRL+C will also quit, unlike `vi(1)`, for those who cannot find `Q`.

* Add extended regular expression forward search with buffer wrap-around.

* Add text selection with delete (cut), yank (copy), and put (paste).

* Add delete (cut), yank (copy) with motion.

* Replace start/end of line with goto-column command.

* Add counts for motion commands, delete, and put.

* Replace top/end of file with goto-line command.

* Fix word left to behave like `vi(1)`.

* Add basic UTF-8 support.


### Award Ideas

* When is C the same as a Ruby, when its your 40th Anniversary.

* 40th Ruby Anniversary - What took you so long?

* Most interesting use of accents, eh?

* Best abuse of UTF-8 (or why ASCII doesn't love you any more).

* Best of something not named Eric.

* Best abuse of standards.

* Best je ne sais quoi.

* Best utility 2.0

* Best upgrade.
