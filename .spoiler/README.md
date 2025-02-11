eh - Edit Here - vi(1) the good parts version
=============================================

A minimalist version of `vi(1)`.  It is an example of the "Buffer Gap" method outlined in the [The Craft Of Text Editing](http://www.finseth.com/craft/) used by many Emacs style editors.  (Yep I mixed `vi` and `emacs` in the same paragraph; I'm going to hell for that one.)


Usage
-----

    eh [filename]

Create or read a text file to edit.  Text files consists of lines of printable ASCII text, tabs, or newline characters.  A physical line can be of arbitrary length and is delimited by either a newline or the end of file.  Tab stops are every eight columns.  The behaviour of non-printable characters may vary depending on the implementation of the Curses library, `stty(1)` settings, or terminal emulator.


Commands
--------

The commands are similar, but not the same as `vi(1)`.  Most commands can be prefixed by a repeat count, eg. `5w`, `123G`, `2dw` (`d2w`), or `2d3w` (`d6w`).  Motion commands, optionally prefixed by a count, are those that move the cursor without modifying the buffer.  Some edit commands can be followed by a motion.

    h j k l     Left, down, up, right cursor movement
    H J K L     Page top, page down, page up, page bottom
    ^F ^B       Page down (forward), page up (back).
    b w         Word left, word right
    ^ $         Start and end of line, ie. `0|` or `999|`
    |           Goto column (count) of physical line.
    /ERE        Find first occurence of ERE pattern after the cursor.
    /ERE/REPL   Find ERE and replace.  In the `REPL`, a `$n` where `n` is
                a digit `0..9` is replaced by the nth subexpression of the
                matched text; `$0` is the whole matched text. `\x` is a
                an escape sequence, ie. \a \b \e \f \n \r \t \? or `x`.
    m char      Set a positional mark letter a..z.
    n           Find next occurence of ERE (and replace); see `u`.
    ` char      Goto position of mark `a .. `z or `` (previous).
    ' char      Goto start of line with mark 'a .. 'z or '' (previous).
    G           Goto line (count) number; 1G top of file, G bottom.
    \           Toggle text selection.
    d motion    Delete text selection or region given by motion.
    y motion    Yank (copy) text selection or region given by motion.
    P p         Paste last deleted or yanked text region before or after
                the cursor.
    i a         Insert text mode before or after the cursor, ESC or CTRL+C
                ends insert.  While inserting text, backspace will erase
                the previous character; CTRL+V treats the next character
                as a literal character.
    X x         Delete character before or after cursor, ie. `dh` or `dl`.
    u           Undo last modification, except invert case.
    ~           Invert character case.
    ! motion    Filter a text region through command(s); `!!command` reads only.
    R           Read a file into buffer after cursor.
    W           Write buffer to file.
    Q           Quit.
    ^C          Quit.

Any other key will redraw the screen.


Environment
-----------

* `SHELL` : The user's shell of choice.

* `TERM` : The user's terminal type.  If the environmental variable `TERM` is not set or insufficient then terminate with non- zero exit status.

* `TERMINFO` : The absolute file path of a `terminfo` database.  See `terminfo(5)`.


Exit Status
-----------

- 0     Success
- 1     Insufficient capabilities for `TERM`.
- 2     Read file error


Build & Test
------------

To build simply:

    $ make build

There is a basic regression test:

    $ make test
    $ make test PROG=./prog

Other targets are:

    $ make clean strip size

Note the differences between NetBSD Curses (main development) and Linux & Cygwin NCurses means that the test suite may fail, because of how the two libraries might reorder escape sequence to produce the same visual result.


References
----------

* Craig A. Finseth, "Craft Of Text Editing", 1991  
  Springer-Verlag, ISBN 0-387-97616-7, ISBN 3-3540-97616-7

* Craig A. Finseth, "Craft Of Text Editing", 1999  
  <http://www.finseth.com/craft/>

* Webb Miller, "A Software Tools Sampler", Prentice Hall, 1987  
  ISBN 0-13-822305-X, chapter 5

* Single Unix Specification, Base Specification 2018, vi(1)  
  <https://pubs.opengroup.org/onlinepubs/9699919799/utilities/vi.html>

* Single Unix Specification, Curses issue 7  
  <https://pubs.opengroup.org/onlinepubs/9699909599/toc.pdf>


Bugs & Differences
------------------

* The display of long physical lines that are larger than the terminal screen is untested, so considered undefined.  In computing, its often bad form when functions are more than a screen length; similarly in English writing a paragraph (as a single long line) that occupies a screen is probably bad form too.

* CRLF newlines (DOS, FreeDOS, Windows) and other non-spacing control characters are not visible.  Consider converting newlines using POSIX tools like `awk(1)` or `sed(1)`:

        $ sed -e's/^V^M$//' dos.txt > unix.txt
        $ sed -e's/$/^V^M/' unix.txt > dos.txt

  `^V` is the default `stty(1)` insert literal prefix key.

* Behaviour of `^` is more like `0` in `vi(1)`, try `^w` (roughly similar except on empty / blank lines).

* `[count]i` not supported, try ``maitext\ed`a[count]P``.

* `[count]$` not supported, try `[count]j$`.

* `[count]dd` not supported, try `^[count]dj`.

* `[count]yy` not supported, try `^[count]yj` or `^[count]dju`.
