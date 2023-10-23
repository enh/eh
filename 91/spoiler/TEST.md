ae Testing
==========

Defines
-------

* Row = A physical screen row.
* Short line = A physical line that display less that COLS columns factoring in tabs.
* Long line = A long physical line that can wrap into two or more logical lines or screen rows depending on the terminal width given by COLS factoring in tabs.
* Logical line or line segment = portion of a long line that sits on a screen row.
* BOF = Beginning of file.
* EOF = End of file just after last character or newline.
* BOL = Beginning of line, which can be BOF.
* EOL = End of line at a newline or EOF if no terminating newline.
* TOP = Top of page.
* BOP = Bottom of page.


Startup
-------

* Empty file.  Cursor at TOP with a blank line and `^D` displayed.
* Non-empty file.  Cursor at TOP and N physical lines on the screen.  N < LINES if one or more long physical lines present.


Scrolling
---------

* Short file less than a screen full.

    + Cursor down `j` from BOF to EOF.  Only cursor moves.
    + Cursor up `k` from EOF to BOF.  Only cursor moves.

* Long file two and half screen fulls in length, with some long lines.  File ends with with a newline.

    + Cursor down `j` from BOF to BOP.  Only cursor moves.
    + Cursor down `j` twice from BOP.  Screen scrolls two physical lines.
    + Cursor up `k` to TOP.  Only cursor moves.
    + Cursor up `k` twice from TOP.  Screen scrolls up two physical lines.


Goto Line
---------

* Goto line `<num>G`:

    + where `<num>` is on the screen.  Only cursor moves on screen.
    + where `<num>` is a line number of top of screen.  Screen redraws with target line at TOP.
    + where `<num>` is a line number of bottom of screen.  Screen redraws with target line at TOP.

* Goto BOF `1G`.  Screen redraws with target line at TOP and BOF.
* Cursor down a few lines on the screen, `1G`.  Cursor moves to BOF.
* Goto EOF `G` where file is:

    + newline terminated.  Screen redraws with last line showing at TOP, cursor at EOF, and `^D` displayed on next row.
    + not newline terminated.  Screen redraws with last line showing at TOP, cursor at EOL/EOF, and `^D` displayed on next row.

* Cursor up `k` twice from EOF.  Screen scrolls up two physical lines.


Page Down
---------

* Short file less than a screen full.

    + Page down `J` once.  See `G` results above concerning file newline terminated or not.  This behaviour is not desireable, but acceptable.  Ideally the the screen should not change, just the cursor moves to EOF.

* Long file two and half screen fulls in length, with some long lines.  File ends with with a newline.

    + Page down `J` once.  Screen redraws with TOP being the next physical line following the previous screen page and cursor remains some screen row, though column may have changed depending on length of line or tab.
    + Page down `J` again.  Screen redraws with TOP being the next physical line following the previous screen page, cursor remains some screen row, though column may have changed depending on length of line or tab, and `^D` on row immediately after EOF.


Page Up
-------

* Long file two and half screen fulls in length, with only short lines.  File ends with with a newline.  Each line will occupy one screen row.

    + Page down `J` once.  Screen redraws with TOP being the next physical line following the previous screen page, same cursor row as previous screen, though the column may have changed depending on shorter line length or tab under cursor.
    + Page down `J` again.  Screen redraws with TOP being the next physical line following the previous screen page, , same cursor row as previous screen, though the column may have changed depending on shorter line length or tab under cursor, and `^D` on row immediately after EOF.
    + Page up `K` once.  Screen redraws at the same as first page down, same cursor row as previous screen.
    + Page up `K` again.  Screen redraws with first line at TOP and BOF, same cursor row as previous screen.

* Long file two and half screen fulls in length, with some long lines.  File ends with with a newline.

    + Page down `J` once.  Screen redraws with TOP being the next physical line following the previous screen page.
    + Page down `J` again.  Screen redraws with TOP being the next physical line following the previous screen page and showing the `^D` on row immediately after EOF.
    + Page up `K` once.  Screen redraws at the same as first page down.  `FAIL`
    + Page up `K` again.  Screen redraws with first line at TOP and BOF.


Insert
------

* Empty file.

    + Insert `i` type some text, newline, more text, ESC.  TOP should remain unchanged, newline pushes `^D` down a row.
    
* Non-empty file.

    + Word right `L`, insert `i`, type some text, ESC.  Any text right of cursor should shift as new text inserted.
    + `G`, insert `i` type in a word, newline, next word, ESC.  TOP should remain unchanged, newline pushes `^D` down a row.


Delete
------

* Non-empty file with at least two lines.

    + Delete `x` a character.  Text to the right of cursor shifts left one.
    + End of line `]`, delete `x` the newline.  The next line moves up joining with the end of the current line.
