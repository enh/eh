eh(1)
=====

Name
----

    eh - Edit Here - vi(1) the good parts version


Synopsis
--------

    eh [filename]


Description
-----------

A minimalist version of `vi(1)`.  It is an example of the "Buffer Gap" method outlined in the [The Craft Of Text Editing](http://www.finseth.com/craft/) used by many Emacs style editors.  (Yep I mixed `vi` and `emacs` in the same paragraph; I'm going to hell for that one.)

Create or read a text file to edit.  Text files consists of lines of printable UTF-8 text, tabs, or newline characters.  A physical line can be of arbitrary length and is delimited by either a newline or the end of file.  Tab stops are every eight columns.  The behaviour of non-printable characters may vary depending on the implementation of the Curses library, `stty(1)` settings, or terminal emulator.


Commands
--------

The commands are similar, but not the same as `vi(1)`.  Most commands can be prefixed by a repeat count, eg. `5w`, `123G`, `2dw` (`d2w`), or `2d3w` (`d6w`).  Motion commands, optionally prefixed by a count, are those that move the cursor without modifying the buffer.  Some edit commands can be followed by a motion.

    h j k l     Left, down, up, right cursor movement.
    H J K L     Page top, page down, page up, page bottom.
    ^F ^B       Page down (forward), page up (back).
    b w         Word left, word right.
    ^ $         Start and end of line, ie. `0|` or `999|`.
    |           Goto column (count) of physical line.
    /ERE        Find first occurrence of ERE pattern after the cursor.
    /ERE/REPL   Find ERE and replace.  In the `REPL`, a `$n` where `n` is
                a digit `0..9` is replaced by the nth subexpression of the
                matched text; `$0` is the whole matched text. `\x` is an
                escape sequence, ie. \a \b \e \f \n \r \t \? or `x`.
    m char      Set a positional mark letter a..z.
    n           Find next occurrence of ERE (and replace); see `u`.
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
    U u         Redo or undo one or more edits.
    ~           Invert character case.
    ! motion    Filter a text selection or region through command(s),
                eg. `!fmt -w68`. Or read only the output of a command,
                eg. `!!ls -l`.
    CTRL+X      Toggle hex digits in the range 0..10FFFF or a Unicode
                character left of the cursor.  The Unicode code point
                must be valid and printable.
    R           Read a file into buffer after cursor.
    W           Write buffer to file.
    V           Show build and version.
    Q           Quit.
    CTRL+C      Quit.

    Any other key will redraw the screen.


Environment
-----------

* `SHELL` :  The user's shell of choice.

* `TERM` :  The user's terminal type.  If the environmental variable `TERM` is not set or insufficient then terminate with non-zero exit status.

* `TERMINFO` :  The absolute file path of a `terminfo` database.  See `terminfo(5)`.


Exit Status
-----------

- 0     Success
- 1     Insufficient capabilities for `TERM`.
- 2     Read file error


See Also
--------

`ed(1)`, `ex(1)`, `vi(1)`


Notes
-----

* Has UTF-8 support.

  - Loads UTF-8 files as-is and internally remains UTF-8 (not converteed to `wchar_t` or `char32_t`).
  - UTF-8 input will likely require an intl. keyboard or enabling [US Intl. dead-key keyboard](https://en.wikipedia.org/wiki/QWERTY#US-International) support.  See also [Unicode Input](https://en.wikipedia.org/wiki/Unicode_input).

* The display of long physical lines that are larger than the terminal screen is untested, so considered undefined.

* Control characters, other than TAB and LF, are displayed as highlighted alphabetic characters.

* `u` and `U` do not behave as in historical `vi(1)`; they now provide multi undo and redo respectively.

* CRLF newlines (DOS, Windows) should be converted to LF newlines  Consider converting newlines using SUS tools like `awk(1)` or `sed(1)`:

        $ sed -e's/^V^M$//' dos.txt > unix.txt
        $ sed -e's/$/^V^M/' unix.txt > dos.txt

  `^V` is the default `stty(1)` insert literal prefix key.


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
