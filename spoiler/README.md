ae 1.4.0
========

A minimalist version of `vi(1)`.  It is an example of the "Buffer Gap" method outlined in the [The Craft Of Text Editing](http://www.finseth.com/craft/) used by many Emacs style editors.  (Yep I mixed `vi` and `emacs` in the same paragraph; I'm going to hell for that one.)


Usage
-----

    ae filename

Create or read a text file to edit.  Text files consists of lines of printable ASCII text, tabs, or newline characters.  A physical line can be of arbitrary length and is delimited by either a newline or the end of file.  Tab stops are every eight columns.  The behaviour of non-printable characters may vary depending on the implementation of the Curses library, `stty(1)` settings, or terminal emulator.


Commands
--------

Most commands can be prefixed by a repeat count, eg. `5w`, `123G`, `2dw` (`d2w`), or `2d3w` (`d6w`).  Motion commands, optionally prefixed by a count, are those that move the cursor without modifying the buffer.

    h j k l     Left, down, up, right cursor movement
    H J K L     Page top, page down, page up, page bottom
    b w         Word left, word right
    ^ $         Start and end of line
    |           Goto column (count) of physical line.
    /ERE        Find first occurence of ERE pattern after the cursor.
    /ERE/REPL   Find ERE and replace.  In the `REPL`, a `$n` where `n` is
                a digit `0..9` is replaced by the nth subexpression of the
                matched text; `$0` is the whole matched text. `\x` is a
                an escape sequence, ie. \a \b \e \f \n \r \t \? or `x`.
    m char      Set a mark letter `a..z` or `.
    n           Find next occurence of ERE (and replace); see `u`.
    ` char      Goto position of mark `a..z` or ` (previous).
    ' char      Goto start of line with mark `a..z` or ` (previous).
    G           Goto line (count) number; 1G top of file, G bottom.
    d motion    Delete text region given by motion.
    y motion    Yank (copy) text region given by motion.
    P p         Paste last deleted or yanked text region before or after
                the cursor.
    i a         Insert text mode before or after the cursor, ESC ends insert.
    X x         Delete character before or after cursor, ie. `dh` or `dl`.
    u           Undo last modification, except invert case.
    ~           Invert character case.
    ! motion    Filter a text region through command(s); `!!command` reads only.
    R           Read a file into buffer after cursor.
    W           Write buffer to file.
    Q           Quit.

Any other key will redraw the screen.


Exit Status
-----------

- 0 success (or missing filename)
- 1 insufficient TERM
- 2 read file error


Build & Test
------------

To build simply:

    $ make build

There is a basic regression test:

    $ make test
    $ make test PROG=./ae-c89

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

* POSIX 1003.2b Draft 11.1 ex & vi, Feb 1996, courtesy of Keith Bostic

* Single Unix Specification, Base Specification 2018, vi(1)  
  <https://pubs.opengroup.org/onlinepubs/9699919799/utilities/vi.html>

* Single Unix Specification, Curses issue 7  
  <https://pubs.opengroup.org/onlinepubs/9699909599/toc.pdf>


Bugs & Differences
------------------

* The display of long physical lines that are larger than the terminal screen is untested, so considered undefined.  In computing, its often bad form when functions are more than a screen length; similarly in English writing a paragraph (as a single long line) that occupies a screen is probably bad form too.

* CRLF newlines (DOS, Windows) and other non-spacing control characters are not visible.  Consider converting newlines using POSIX tools like `awk(1)` or `sed(1)`:

    sed -e's/^M$//' dos.txt > unix.txt
    sed -e's/$/^M/' unix.txt > dos.txt

* Behaviour of `^` is more like `0` in `vi(1)`, try `^w` (roughly similar except on empty / blank lines).

* `[count]i` not supported, try ``maitext\ed`a[count]P``.

* `[count]$` not supported, try `[count]j$`.

* `[count]dd` not supported, try `^[count]dj`.

* `[count]yy` not supported, try `^[count]yj` or `^[count]dju`.


Notes
-----

Version 1.0.0
* Original 1990 K&R `ae.c`.

Version 1.1.0
* `ae-c89.c` is the C89 version of K&R `ae.c`.
* Address Curses library differences.
* Address compiler warnings.

Version 1.2.0
* Replaced `t` and `b` commands with `G`, while slightly more code, it provides more functionality.
* Fixed the behaviour of `H` word left.
* Minor code compaction.

Version 1.3.0
* Resolves original 1991 (ae91-1.0.0) issues with scrolling and paging a file with long physical lines.
* Rename word left `H` and word right `L` commands to `b` and `w` more like `vi(1)`.
* Rename line begin `[` and line end `]` commands to `^` and `$` more like `vi(1)`.
* Add page top `H` and bottom `L` commands.
* Add `/` and `n` commands.
* Add set `m` and goto mark `` ` `` commands.
* Add delete `d motion` (eg. 3dw = d2w, 2d3w = d6w), yank `y motion`, paste before `P`, and undo last `u`.
* Add a regression test suite.

Version 1.4.0
* Adjust marks after the cursor when the buffer is altered to maintain relative position.
* Add `'` goto marked line, delete before `X`, paste after `p`, append `a`, read file `R` commands.
* Switch from cbreak() to raw() input; handle INTR `^C`, LNEXT `^V`, and SUSP `^Z`.
* INTR `^C` can leave insert mode same as `vi(1)` or quit (for those who don't read documentation).
* Fix `[count]p` to ensure sufficent space before pasting and fix undo of last `[count]p` command.
* ASCII control characters are displayed as highlighted characters.
* Support search & replace `/ERE/REPL/`.
* Add filter `!` text region through external program.
