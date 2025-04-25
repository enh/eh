48c574c3-3916-46c6-a5e1-b7a6bb760ef8 Slut 6
-------------------------------------------

### Building

Simply type `make` to build.  There are two macros that can be customised:

* `BUF` to set the buffer size; default 128KB.

* `MODE` to set the file creation mode; default 0600.


### What's Interesting.

* This entry requires Curses.

* `prog` can view its own source.

* The source is UTF-8.  When are `é` `ë` `ê` `è` not same as `e`?

* This is an update with bug fixes of a previous IOCCC winner, with permission of the author.

* A regression test suite is available on demand.  The test suite requires `tic(1)` to build the specialised test terminal entry.

* Original unobfuscated more fully featured source available (search/replace, pipe-filter selection, markers, insert file, edit buffer grows, literal character input).

* The `iocccsize(1)` tool only counts bytes, not characters, ie. UTF-8 multibyte characters such that some characters cost 2, 3, or 4 bytes, thus penalising UTF-8 entries.

   > UTF-8 demands equal rights and respect as real characters!  The dominance of slim ASCII must end!  We are the UTF-8 People's Front for Freedom, Equality, and Brotherhood (oh that's been used already) - We are the UTF-8 Countless Tourist Defence Farce for One Voice, One World, One Character Set.


#### Updates

* Refactor page framing to address original issue of scrolling and paging up/down when the file contains long lines.

* CTRL+C will exit insert mode as with `vi(1)`; CTRL+C will also quit, unlike `vi(1)`, for those who cannot find `Q`.

* Add extended regular expression forward search with buffer wrap-around.

* Add text selection with delete (cut), yank (copy), and put (paste).

* Add delete (cut), yank (copy) with motion.

* Replace start/end of line with goto-column command.

* Add counts for motion commands, delete, and put.

* Replace top/end of file with goto-line command.

* Fix word left to behave like `vi(1)`.

* Add upper/lower case inverting.

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


- - -

eh(1)
=====

NAME
----

    eh - Edit Here - vi(1) the good parts version


SYNOPSIS
--------

    eh filename


DESCRIPTION
-----------

A minimalist version of `vi(1)`.  It is an example of the "Buffer Gap" method outlined in the [The Craft Of Text Editing](http://www.finseth.com/craft/) used by many Emacs style editors.  (Yep I mixed `vi` and `emacs` in the same paragraph; I'm going to hell for that one.)

Create or read a text file to edit.  Text files consists of lines of printable UTF-8 text, tabs, or newline characters.  A physical line can be of arbitrary length and is delimited by either a newline or the end of file.  Tab stops are every eight columns.  The behaviour of non-printable characters may vary depending on the implementation of the Curses library, `stty(1)` settings, or terminal emulator.


COMMANDS
--------

The commands are similar, but not the same as `vi(1)`.  Most commands can be prefixed by a repeat count, eg. `5w`, `123G`, `2dw` (`d2w`), or `2d3w` (`d6w`).  Motion commands, optionally prefixed by a count, are those that move the cursor without modifying the buffer.  Some edit commands can be followed by a motion.

    h j k l     Left, down, up, right cursor movement.
    H J K L     Page top, page down, page up, page bottom.
    b w         Word left, word right.
    |           Goto column (count) of physical line.
    /ERE        Find first occurrence of ERE pattern after the cursor.
    n           Find next occurrence of ERE (and replace); see `u`.
    G           Goto line (count) number; 1G top of file, G bottom.
    \           Toggle text selection.
    d motion    Delete text selection or region given by motion.
    y motion    Yank (copy) text selection or region given by motion.
    P           Paste last deleted or yanked text region before or after
                the cursor.
    i           Insert text mode before the cursor, ESC or CTRL+C ends insert.
    x           Delete character after cursor, ie. `dl`.
    W           Write buffer to file.
    Q           Quit.
    CTRL+C      Quit.

    Any other key will redraw the screen.


ENVIRONMENT
-----------

* `TERM` : The user's terminal type.  If the environmental variable `TERM` is not set or insufficient then terminate with non- zero exit status.

* `TERMINFO` : The absolute file path of a `terminfo` database.  See `terminfo(5)`.


EXIT STATUS
-----------

- 0     Success
- 1     Insufficient capabilities for `TERM`.
- 2     Read file error


SEE ALSO
--------

`ed(1)`, `ex(1)`, `vi(1)`


NOTES
-----

* Has UTF-8 support.

  - Loads UTF-8 files as-is and internally remains UTF-8 (not converteed to `wchar_t`).
  - UTF-8 input will likely require an intl. keyboard or enabling [US Intl. dead-key keyboard](https://en.wikipedia.org/wiki/QWERTY#US-International) support.  See also [Unicode Input](https://en.wikipedia.org/wiki/Unicode_input).

* The display of long physical lines that are larger than the terminal screen is untested, so considered undefined.

* CRLF newlines (DOS, Windows) and other non-spacing control characters are not visible.  Consider converting newlines using SUS tools like `awk(1)` or `sed(1)`:

        $ sed -e's/^V^M$//' dos.txt > unix.txt
        $ sed -e's/$/^V^M/' unix.txt > dos.txt

  `^V` is the default `stty(1)` insert literal prefix key.


REFERENCES
----------

* Craig A. Finseth, "Craft Of Text Editing", 1999  
  <http://www.finseth.com/craft/>

* Single Unix Specification, Curses issue 7  
  <https://pubs.opengroup.org/onlinepubs/9699909599/toc.pdf>

* Single Unix Specification, Base Specification 2018, vi(1)  
  <https://pubs.opengroup.org/onlinepubs/9699919799/utilities/vi.html>
