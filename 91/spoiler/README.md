ae 1.3.0
========

A minimalist version of `vi(1)`.  It is an example of the "Buffer Gap" method outlined in the [The Craft Of Text Editing](http://www.finseth.com/craft/) used by many Emacs style editors.


Usage
-----

    ae [filename]

Read or create a text file; default filename is `a.txt`.  Text files consists of lines of printable text, tabs, or newline characters.  A physical line can be of arbitrary length and is delimited by either a newline or the end of file.  Tab stops are every eight columns.  The behaviour of non-printable characters may vary depending on the implementation of the Curses library, `stty(1)` settings, or terminal emulator.


Commands
--------

    h j k l   left, down, up, right cursor movement
    H J K L   page top, page down, page up, page bottom
    b w       word left, word right
    [ ]       beginning and end of line
    / n       find ERE pattern, find next occurrence
    G         goto line number; 1G top of file, G bottom
    i         enter insert mode, FF or ESC to quit
    x         delete character under the cursor
    W         write buffer to file
    Q         quit

Any other key will redraw the screen.


Exit Status
-----------

- 0 success
- 1 error: insufficient TERM; read file error


Build & Test
------------

To build simply:

    $ make build

There is a basic regression test based on the [testing notes](./TEST.md) for manual visual testing.

    $ make test
    $ make test PROG=./ae-c89

Other targets are:

    $ make clean strip size


Bugs
----

* The display of long physical lines that are larger than the terminal screen is undefined.  Given historical terminal dimensions of 80x24 = 1920 ASCII characters, a line longer than 1918 characters will have issues.  In computing, its often bad form when functions are more than a screen length; similarly in English writing a paragraph (as a single long line) that occupies a screen is probably bad form too.


Notes
-----

`ae-c89.c` is the C89 update of `ae.c` using pointers into the buffer; version 1.1.0 is the conversion with some changes to address compiler warnings and Curses library differences.

`ae-alt.c` is `ae-c89.c` converted to use offsets into the buffer inspired by a derivative version from https://github.com/the8thbit/e that raised a question as to which method, given today's optimising compilers, yields the smallest binary.

Comparing the stripped binary sizes between `ae-c89` and `ae-alt` could differ from the `prog` and `prog-alt`, because of size differences between `off_t` and `ptrdiff_t` (8 bytes) versus `int` (4 bytes) as defined on NetBSD 64-bit.

Version 1.2.0 found additional places to compact the code further and also replaced `t` and `b` commands with `G`, while slightly more code, it provides more functionality.  Also fixed the behaviour of `H` word left.

Version 1.3.0 resolves issues with scrolling and paging a file with long physical lines.  Renames `H`, `L` (word left, right) commands to `b` and `w` more in line with `vi(1)`; add `H` and `L` page top and bottom commands.  Add `/` and `n` commands. Add a regression test suite.


References
----------

* Craig A. Finseth, "Craft Of Text Editing", 1991  
  Springer-Verlag, ISBN 0-387-97616-7, ISBN 3-3540-97616-7

* Craig A. Finseth, "Craft Of Text Editing", 1999  
  <http://www.finseth.com/craft/>

* Webb Miller, "A Software Tools Sampler", Prentice Hall, 1987  
  ISBN 0-13-822305-X, chapter 5
