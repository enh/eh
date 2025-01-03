### Building

Simply type `make` to build.  There are two macros that can be customised:

* `BUF` to set the buffer size; default 128KB.

* `MODE` to set the file creation mode; default 0600.


### What's Interesting.

* This entry requires Curses.

* This is an update with bug fixes of a previous IOCCC winner.

* The source is UTF-8.  When are `é` `ë` `ê` `è` not same as `e`?

* A regression test suite is available on demand.  See `make test`.  The test suite will likely require `tic(1)` to build the specialised test terminal entry.

* Original unobfuscated more fully featured source available.


#### Updates

* Refactor page framing to address original issue of scrolling and paging up/down when the file contains long lines.

* CTRL+C will exit insert mode as with `vi(1)`; CTRL+C will quit, unlike `vi(1)`.

* Add extended regular expression forward search with buffer wrap-around.

* Add text selection with delete (cut), yank (copy), and put (paste).

* Add delete (cut), yank (copy) with motion.

* Replace start/end of line with goto-column command.

* Add counts for motion commands, delete, and put.

* Replace top/end of file with goto-line command.

* Fix word left to behave like `vi(1)`.

* Add upper/lower case inverting.


### Award Ideas

* 40th Ruby Anniversary - What Took You So Long?  

* Most interesting use of accents, eh?

* Best of something not named Eric.

* Best abuse of standards.

* Best je ne sais quoi.

* Best winner upgrade.


### The Man Page
```
eh(1)                                                                   eh(1)

NAME

    eh - Edit Here - vi(1) the good parts version


SYNOPSIS

    eh filename


DESCRIPTION

    Create or read a text file to edit.  Text files consists of lines of
    printable ASCII text, tabs, or newline characters.  A physical line
    can be of arbitrary length and is delimited by either a newline or
    the end of file.  Tab stops are every eight columns.  The behaviour
    of non-printable characters may vary depending on the implementation
    of the Curses library, `stty(1)` settings, or terminal emulator.


COMMANDS

    The commands are similar, but not the same as `vi(1)`.  Most commands
    can be prefixed by a repeat count, eg. `5w`, `123G`, `2dw` (`d2w`),
    or `2d3w` (`d6w`).  Motion commands, optionally prefixed by a count,
    are those that move the cursor without modifying the buffer.  Some
    edit commands can be followed by a motion.

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
    u           Undo last modification, except invert case.
    ~           Invert character case.
    W           Write buffer to file.
    Q           Quit.
    CTRL+C      Quit.

    Any other key will redraw the screen.


ENVIRONMENT

    TERM        The user's terminal type.  If the environmental variable
                `TERM` is not set or insufficient then terminate with non-
                zero exit status.

    TERMINFO    The absolute file path of a `terminfo` database.  See
                `terminfo(5)`.


EXIT STATUS

    0           Success
    1           Insufficient capabilities for `TERM`.
    2           Read file error


NOTES

* The display of long physical lines that are larger than the terminal
  screen is untested, so considered undefined.

* CRLF newlines (DOS, FreeBSD Windows) and other non-spacing control
  characters are not visible.  Consider converting newlines using SUS
  tools like `awk(1)` or `sed(1)`:

    sed -e's/^M$//' dos.txt > unix.txt
    sed -e's/$/^M/' unix.txt > dos.txt


REFERENCES

* Craig A. Finseth, "Craft Of Text Editing", 1999
  <http://www.finseth.com/craft/>

* Single Unix Specification, Curses issue 7
  <https://pubs.opengroup.org/onlinepubs/9699909599/toc.pdf>

* Single Unix Specification, Base Specification 2018, vi(1)
  <https://pubs.opengroup.org/onlinepubs/9699919799/utilities/vi.html>
```

