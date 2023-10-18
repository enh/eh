ae 1.2.0
========

A minimalist version of `vi(1)`.  It is an example of the "Buffer Gap" method outlined in the [The Craft Of Text Editing](http://www.finseth.com/craft/) used by many Emacs style editors.


Usage
-----

    ae file

Text files consists of lines of printable text or tab characters.  A line can be of arbitrary length and is delimited by either a newline or the end of file.  Carriage return is mapped to newline on input and ignored on output.  Tab stops are every eight columns.  Non-printable characters may have unpredictable results depending on the implementation of the Curses library.


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
- 2 missing filename


Bugs
----

* `ae` will display a file with long lines (greater than the terminal width), but has trouble scrolling the screen containing long lines.  Paging up and down should work correctly, however.


Notes
-----

`ae-c89.c` is the C89 update of `ae.c` using pointers into the buffer; version 1.1.0 is the conversion with some changes to address compiler warnings and Curses library differences.

`ae-alt.c` is `ae-c89.c` converted to use offsets into the buffer inspired by a derivative version from https://github.com/jarnosz/e that raised a question as to which method, given today's optimising compilers, yields the smallest binary.

Comparing the stripped binary sizes between `ae-c89` and `ae-alt` could differ from the `prog` and `prog-alt`, because of size differences between `off_t` and `ptrdiff_t` (8 bytes) versus `int` (4 bytes) as defined on NetBSD 64-bit.

    elf$ iocccsize -v1 prog.c
    1393 2878 117
    elf$ iocccsize -v1 prog-alt.c
    1407 2881 117
    elf$ ls -l ae ae-c89 ae-alt prog* ../ant*
    -rwxr-x---  1 achowe  users  12080 Oct 20 18:00 ../ant
    -rwx------  1 achowe  users   1478 Oct 17 08:17 ../ant.c
    -rwxr-x---  1 achowe  users  10384 Oct 20 10:04 ae
    -rwxr-x---  1 achowe  users  10464 Oct 20 10:04 ae-alt
    -rwxr-x---  1 achowe  users  10456 Oct 20 10:04 ae-c89
    -rwxr-x---  1 achowe  users  10328 Oct 20 10:04 prog
    -rwxr-x---  1 achowe  users  10320 Oct 20 10:04 prog-alt
    -rw-r-----  1 achowe  users   2881 Oct 20 10:04 prog-alt.c
    -rw-r-----  1 achowe  users   2878 Oct 20 10:04 prog.c

Oddly enough the original `ant` stripped binary is larger than the revised versions.  This appears to be a reflection of how ` gcc` handles K&R versus C89 source; I suspect the C89 version (1.1.0) being more clearly and strongly typed allows the compiler to make better optimisations for `-Os`.

Version 1.2.0 found additional places to compact the code further and also replaced `t` and `b` commands with `G`, while slightly more code, it provides more functionality.  Also fixed the behaviour of `H` word left.


References
----------

* Craig A. Finseth, "Craft Of Text Editing", 1991
  Springer-Verlag, ISBN 0-387-97616-7, ISBN 3-3540-97616-7

* Craig A. Finseth, "Craft Of Text Editing", 1999
  <http://www.finseth.com/craft/>
