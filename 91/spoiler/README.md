ae 1.2.0
========

A minimalist version of `vi(1)`.  It is an example of the "Buffer Gap" method outlined in the [The Craft Of Text Editing](http://www.finseth.com/craft/) used by many Emacs style editors.


Usage
-----

    ae [filename]

Read or create a text file; default filename is `a.txt`.  Text files consists of lines of printable text, tabs, or newline characters.  A physical line can be of arbitrary length and is delimited by either a newline or the end of file.  Tab stops are every eight columns.  Carriage return and newline behaviour is based on the `stty(1)` input mode settings or terminal emulator or both.  The behaviour of other non-printable characters may be undefined depending on the implementation of the Curses library or `stty(1)` settings.


Commands
--------

    h j k l   left, down, up, right cursor movement
    H J K L   word left, page down, page up, word right
    [ ]       beginning and end of line
    G         goto line number; 1G top of file, G bottom
    i         enter insert mode, FF or ESC to quit
    x         delete character under the cursor
    W         write buffer to file
    Q         quit

Any other key will redraw the screen.


Exit Status
-----------

- 0 success
- 1 file read error


Bugs
----

* See [testing notes](./TEST.md) for manual visual testing instructions and expected behaviour.  Failures indicated by a `FAIL` tag.

* `ae` will display a file with long physical lines (greater than the terminal width), but has trouble paging up where the previous page contains some long lines.  Scrolling up / down and paging down work correctly (an improvement over the original 1991 version).  Paging up is an issue, because it counts back `LINES` physical line units, not logical line units.  See [testing notes](./TEST.md).

* The display of long physical lines that are larger than the terminal screen is undefined.  Given historical terminal dimensions of 80x24 = 1920 ASCII characters, a line longer than 1918 characters will have issues.  In computing, its often bad form when functions are more than a screen length; similarly in English writing a paragraph (as a single long line) that occupies a screen is probably bad form too.


Notes
-----

`ae-c89.c` is the C89 update of `ae.c` using pointers into the buffer; version 1.1.0 is the conversion with some changes to address compiler warnings and Curses library differences.

`ae-alt.c` is `ae-c89.c` converted to use offsets into the buffer inspired by a derivative version from https://github.com/jarnosz/e that raised a question as to which method, given today's optimising compilers, yields the smallest binary.

Comparing the stripped binary sizes between `ae-c89` and `ae-alt` could differ from the `prog` and `prog-alt`, because of size differences between `off_t` and `ptrdiff_t` (8 bytes) versus `int` (4 bytes) as defined on NetBSD 64-bit.

Oddly enough the original `ant` stripped binary is larger than the revised versions.  This appears to be a reflection of how ` gcc` handles K&R versus C89 source; I suspect the C89 version (1.1.0) being more clearly and strongly typed allows the compiler to make better optimisations for `-Os`.

Version 1.2.0 found additional places to compact the code further and also replaced `t` and `b` commands with `G`, while slightly more code, it provides more functionality.  Also fixed the behaviour of `H` word left.


References
----------

* Craig A. Finseth, "Craft Of Text Editing", 1991
  Springer-Verlag, ISBN 0-387-97616-7, ISBN 3-3540-97616-7

* Craig A. Finseth, "Craft Of Text Editing", 1999
  <http://www.finseth.com/craft/>
