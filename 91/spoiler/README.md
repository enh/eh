ae 1.3.0
========

A minimalist version of `vi(1)`.  It is an example of the "Buffer Gap" method outlined in the [The Craft Of Text Editing](http://www.finseth.com/craft/) used by many Emacs style editors.


Usage
-----

    ae filename

Create or read a text file to edit.  Text files consists of lines of printable ASCII text, tabs, or newline characters.  A physical line can be of arbitrary length and is delimited by either a newline or the end of file.  Tab stops are every eight columns.  The behaviour of non-printable characters may vary depending on the implementation of the Curses library, `stty(1)` settings, or terminal emulator.


Commands
--------

Most commands can be prefixed by a repeat count.  Commands that do not support (ignore) count are `[`, `]`, `/`, `i`, `W`, and `Q`.  Motion commands are those that move the cursor without modifying the buffer.

    h j k l   left, down, up, right cursor movement
    H J K L   page top, page down, page up, page bottom
    b w       word left, word right
    ^ $       beginning and end of line
    / n       find ERE pattern, find next occurrence
    m char    set a mark letter a..z
    ` char    goto previously set mark a..z
    G         goto line (count) number; 1G top of file, G bottom
    d motion  delete text region given by motion
    P         paste last deleted text region before the cursor
    i         insert text mode before the cursor, ESC to quit
    x         delete character under the cursor, ie. dl
    u         undo last modification, except invert case
    ~         invert character case
    W         write buffer to file
    Q         quit

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

There is a basic regression test based:

    $ make test
    $ make test PROG=./ae-c89

Other targets are:

    $ make clean strip size


References
----------

* Craig A. Finseth, "Craft Of Text Editing", 1991  
  Springer-Verlag, ISBN 0-387-97616-7, ISBN 3-3540-97616-7

* Craig A. Finseth, "Craft Of Text Editing", 1999  
  <http://www.finseth.com/craft/>

* Webb Miller, "A Software Tools Sampler", Prentice Hall, 1987  
  ISBN 0-13-822305-X, chapter 5

* POSIX 1003.2b Draft 11.1 ex & vi, Feb 1996, courtesy of Keith Bostic

* Single Unix Specification, Base Specification 2018  
  <https://pubs.opengroup.org/onlinepubs/9699919799.2018edition/>

* Single Unix Specification, Curses issue 7  
  <https://pubs.opengroup.org/onlinepubs/9699909599/toc.pdf>


Bugs & Differences
------------------

* The display of long physical lines that are larger than the terminal screen is untested, so considered undefined.  In computing, its often bad form when functions are more than a screen length; similarly in English writing a paragraph (as a single long line) that occupies a screen is probably bad form too.

* CRLF newlines (DOS, Windows) and other non-spacing control characters are not visible.  Consider converting newlines using POSIX tools like `awk(1)` or `sed(1)`:

    sed -e's/^M$//' dos.txt > unix.txt
    sed -e's/$/^M/' unix.txt > dos.txt

* Marks set beyond the point of an insert or delete operation are not adjusted (maintained), so they will point to out of date locations.

* Behaviour of `^` is more like `0` in `vi(1)`, try `^w` (roughly similar except on empty / blank lines).

* `[count]i`not supported, try ``maitext\ed`a[count]P``.

* `[count]$` not supported, try `[count]j$`.

* `[count]dd` not supported, try `^[count]dj`.


Notes
-----

Version 1.1.0
* `ae-c89.c` is the C89 version of K&R `ae.c`.
* Address Curses library differences.
* Address compiler warnings.

Version 1.2.0
* Replaced `t` and `b` commands with `G`, while slightly more code, it provides more functionality.
* Fixed the behaviour of `H` word left.
* Minor code compaction.
* `ae-alt.c` is `ae-c89.c` converted to use offsets into the buffer inspired by a derivative version from https://github.com/the8thbit/e that raised a question as to which method, given today's optimising compilers, yields the smallest binary.  Comparing the stripped binary sizes between `ae-c89` and `ae-alt` could differ from the `prog` and `prog-alt`, because of size differences between `off_t` and `ptrdiff_t` (8 bytes) versus `int` (4 bytes) as defined on NetBSD 64-bit.

Version 1.3.0
* Resolves original 1991 (ae91-1.0.0) issues with scrolling and paging a file with long physical lines.
* Rename word left `H` and word right `L` commands to `b` and `w` more like `vi(1)`.
* Rename line begin `[` and line end `]` commands to `^` and `$` more like `vi(1)`.
* Add page top `H` and bottom `L` commands.
* Add `/` and `n` commands.
* Add set `m` and goto mark `` ` `` commands.
* Add delete `d motion` (eg. 3dw = d2w, 2d3w = d6w), paste before `P`, and undo last `u`.
* Add a regression test suite.
