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


Testing
-------

This document contains a mix of short and long physical lines.

* Row = A physical screen row.
* Short line = A physical line of text that display less that `COLS` columns factoring in tabs.
* Long line = A physical line of text that can wrap onto two or more logical lines (screen rows) depending on the terminal width given by `COLS` factoring in tabs.
* Logical line = portion of a long line that fits on a single screen row.
* BOF = Beginning of file.
* EOF = End of file just after last character or newline.
* BOL = Beginning of line, which can be BOF.
* EOL = End of line at a newline or EOF if no terminating newline.
* TOP = Top of page.
* BOP = Bottom of page.


### Terminal Dimensions

Historical serial CRT terminals were 80 columns by 24 rows.  Later serial terminals were 80x25 to make room for a status line.  X Windows, Windows, and Mac systems have terminal emulators (typically `xterm`, `ansi`, or `vt100`) that can be resized on the fly.  Typically the column width is a multiple of eight (8) to account for `TAB` stops.  The terminal dimensions can be found with `stty -a` or check the `TERM` definition (see `/usr/share/misc/terminfo`) or both.

Curses library supports the environment variables `TERM`, `LINES`, and `COLUMNS`.  It possible to temporarily set `LINES` or `COLUMNS` or both smaller than the terminal window size for easier testing and debugging, eg.

    $ LINES=10 ./ae file
