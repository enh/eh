/*
 * Edit Here
 *
 * Copyright 2024, 2025 by Anthony C Howe.  All rights reserved.
 */

#define EXT

#include <ctype.h>
#include <assert.h>
#include <curses.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#ifdef EXT
#include <locale.h>
#include <wctype.h>
#include <signal.h>
#include <sys/wait.h>
#else /* EXT */
#endif /* EXT */

#ifndef BUF
# define BUF		(128*1024)
#endif
#ifndef MODE
# define MODE		0600
#endif

#define CTRL_C		'\003'
#define CTRL_V		'\026'
#define CTRL_Z		'\032'
#define ESC		'\033'

#define CHANGED		'*'
#define NOCHANGE	' '
#define MARKS		27
#define MAX_COLS	999
#define TABWIDTH	8
#define TABSTOP(col)	(TABWIDTH - ((col) & (TABWIDTH-1)))

#define TOP_LINE	1
#define ROWS		(LINES-TOP_LINE)

#define ALL_CMDS	99

#ifdef EXT
#define MATCHES		10
#define MOTION_CMDS	18

static char chg = NOCHANGE;
static int cur_row, cur_col, count, ere_dollar_only;
static char *filename, *scrap, *replace;
static char *buf, *gap, *egap, *ebuf, *ugap, *uegap;
static const char ins[] = "INS", cmd[] = "   ", *mode = cmd;
static off_t here, page, epage, uhere, match_length, scrap_length, marks[MARKS], marker = -1;
static regex_t ere;
#else /* EXT */
#define MATCHES		1
#define MOTION_CMDS	14

static int cur_row, cur_col, count, ere_dollar_only;
static char *filename, *scrap;
static char buf[BUF], *gap = buf, *egap, *ebuf, *ugap = buf, *uegap;
static off_t here, page, epage, uhere, match_length, scrap_length, marker = -1;
static regex_t ere;
#endif /* EXT */

void
getcmd(int);

/*
 * The original prog.c for the IOCCC conformed to the 1536 bytes of
 * source size rule.  Not yet sure if this version will still conform,
 * though it should for more current versions of the size rule (4096,
 * 2503).
 *
 *	0 <= off_t <= file size	eg. here (the cursor)
 *
 *	buf <= char * <= ebuf	eg. gap (start of the hole)
 */

/*
 *	The following assertions must be maintained.
 *
 *	o  buf <= gap <= egap <= ebuf
 *		If gap == egap then the buffer is full.
 *
 *	o  point = ptr(here) and point < gap or egap <= point
 *
 *	o  page <= here < epage
 *
 *	o  0 <= here <= pos(ebuf) <= BUF
 *
 *
 *	Memory representation of the file:
 *
 *		low	buf  -->+----------+
 *				|  front   |
 *				| of file  |
 *			gap  -->+----------+<-- character not in file
 *				|   hole   |
 *			egap -->+----------+<-- character in file
 *				|   back   |
 *				| of file  |
 *		high	ebuf -->+----------+<-- character not in file
 *
 *
 *	point & gap
 *
 *	The Point is the current cursor position while the Gap is the
 *	position where the last edit operation took place. The Gap is
 *	ment to be the cursor but to avoid shuffling characters while
 *	the cursor moves it is easier to just move a pointer and when
 *	something serious has to be done then you move the Gap to the
 *	Point.
 */

/*
 * Translate a buffer offset into a cursor offset,
 * where the gap size has to be factored out.
 */
off_t
pos(const char *s)
{
	assert(buf <= s && s <= ebuf);
	return s-buf - (s < egap ? 0 : egap-gap);
}

/*
 * Translate a cursor offset into a buffer offset,
 * where the gap size has to be factored in.
 */
char *
ptr(off_t cur)
{
	assert(0 <= cur && cur <= pos(ebuf));
	return buf+cur + (buf+cur < gap ? 0 : egap-gap);
}

#ifdef EXT
/**
 * Alternative to mblen() that assumes ch is a the start byte of
 * UTF-8 multibyte character.  Returns the multibyte length; if
 * ch is a continuation byte, it returns 1, so a scanner will
 * move across a partial multibyte character to the start of the
 * next sequence.
 */
int
mblength(int ch)
{
//	return (1+(ch > 193)+(ch > 223)+(ch > 239)) * (ch < 128 || (193 < ch && ch < 245));
	return 1+(ch > 193)+(ch > 223)+(ch > 239);
}

off_t
nextch(off_t cur)
{
	/* Advance to next UTF-8 start byte.  Do not read past eof. */
	return cur + (cur < pos(ebuf) ?  mblength(*ptr(cur)) : 0);
}

off_t
prevch(off_t cur)
{
	/* Find UTF-8 start byte skipping continuation bytes. */
	while (0 < cur && (192 & *ptr(--cur)) == 128) {
		;
	}
	assert(0 <= cur);
	return cur;
}
#else /* EXT */
#define nextch(cur)	((cur)+((cur)<pos(ebuf)))
#define prevch(cur)	((cur)-(0<(cur)))
#endif /* EXT */

/* -60B undo() */
void
setundo(void)
{
	ugap = gap;
	uegap = egap;
	uhere = here;
}

#ifdef EXT
void
adjmarks(void)
{
	off_t p = pos(egap);
	off_t n = /* insert, paste */(gap-ugap) - /* delete */(egap-uegap);
	for (int i = 0; i < MARKS; i++) {
		if (p < marks[i]) {
			marks[i] += n;
		}
	}
}
#else /* EXT */
#define adjmarks()
#endif /* EXT */

void
undo(void)
{
	char *m = ugap, *n = uegap;
	off_t p = uhere;
	setundo();
	here = p;
	egap = n;
	gap = m;
	adjmarks();
	/* Force display() to reframe, ie. 1GdGu fails. */
	epage = here+1;
}

void
movegap(off_t cur)
{
	assert(0 <= cur && cur <= pos(ebuf));
#ifdef FAST_MOVE
	ssize_t len = gap - buf - cur;
	if (len < 0) {
		/* Shift data down, moving gap up to cursor. */
		(void) memcpy(gap, egap, -len);
		egap -= len;
		gap -= len;
	} else {
		/* Shift data up, moving gap down to cursor. */
		gap -= len;
		egap -= len;
		(void) memmove(egap, gap, len);
	}
#else /* FAST_MOVE */
	char *p = ptr(cur);
	while (p < gap) {
		*--egap = *--gap;
	}
	while (egap < p) {
		*gap++ = *egap++;
	}
#endif /* FAST_MOVE */
	assert(buf <= gap && gap <= egap && egap <= ebuf);
	setundo();
}

#ifdef EXT
void
growgap(size_t min)
{
	char *xbuf;
	assert(buf <= gap && gap <= egap && egap <= ebuf);
	/* The gap is used for field input and should be at least as
	 * large as the screen width.
	 */
	if ((size_t)(egap-gap) < min) {
		/* Remember current gap location. */
		off_t xhere = pos(egap);
		ptrdiff_t buflen = ebuf-buf;
		/* Append the new space to the end of the gap. */
		movegap(pos(ebuf));
		off_t gap_off = pos(gap);
		if (NULL == (xbuf = realloc(buf, buflen+BUF))) {
			/* Bugger.  Don't exit, allow user to write file. */
			(void) beep();
			return;
		}
		egap = ebuf = xbuf+buflen+BUF;
		gap = xbuf+gap_off;
		buf = xbuf;
		/* Restore gap's previous location. */
		movegap(xhere);
	}
}
#else /* EXT */
#define growgap(n)
#endif /* EXT */

int
charwidth(const char *s, int col)
{
#ifdef EXT
	wchar_t wc;
	(void) mbtowc(&wc, s, 4);
	return wc == '\t' ? TABSTOP(col) : (col = wcwidth(wc)) < 0 ? 1 : col;
#else /* EXT */
	return *s == '\t' ? TABSTOP(col) : 1;
#endif /* EXT */
}

/*
 * Return the physical BOL or BOF containing cur.
 */
off_t
bol(off_t cur)
{
	while (0 < cur && *ptr(--cur) != '\n') {
		;
	}
	assert(-1 <= cur);
	return (cur+1) * (0 < cur);
}

/*
 * Return offset of column position, newline (EOL), or EOF; otherwise
 * if maxcol is way larger than the terminal width, eg. 999 (assumes
 * that you're not using an IMAX theatre screen for your terminal),
 * just EOL or EOF.
 */
off_t
col_or_eol(off_t cur, int col, int maxcol)
{
	char *p;
	while (col < maxcol && (p = ptr(cur)) < ebuf && *p != '\n') {
		col += charwidth(p, col);
#ifdef EXT
		cur = nextch(cur);
#else /* EXT */
		cur++;
#endif /* EXT */
	}
	assert(0 <= cur && cur <= pos(ebuf));
	return cur;
}

/*
 * Return offset to start of logical line containing offset.
 */
off_t
row_start(off_t cur, off_t offset)
{
	char *p;
	int col = 0;
	off_t mark = cur;
	assert(/* 0 <= cur && cur <= offset &&*/ offset <= pos(ebuf));
#ifdef EXT
	while (cur < offset && (p = ptr(cur = nextch(cur))) < ebuf) {
#else /* EXT */
	while (cur < offset && (p = ptr(++cur)) < ebuf) {
#endif /* EXT */
		col += charwidth(p, col);
		if (COLS <= col) {
			mark = cur;
			col = 0;
		}
	}
	assert(0 <= mark && mark <= cur);
	return mark;
}

/*
 * Return the previous logical BOL or BOF.
 */
off_t
prevline(off_t cur)
{
	off_t s = bol(cur);		/* Current physical line. */
	off_t t = row_start(s, cur);	/* Current logical line. */
	if (s < t) {
		/* Within current physical line, find previous logical line. */
		return row_start(s, t-1);
	}
	/* Previous physical line, find last logical line. */
	return row_start(bol(s-1), s-1);
}

/*
 * Return the next logical EOL or EOF.
 */
off_t
nextline(off_t cur)
{
#ifdef EXT
	return nextch(col_or_eol(cur, cur_col, COLS-1));
#else /* EXT */
	cur = col_or_eol(cur, cur_col, COLS-1);
	return cur + (cur < pos(ebuf));
#endif /* EXT */
}

void
clr_to_eol(void)
{
	(void) printw("%*s", COLS-getcurx(stdscr), "");
}

void
display(void)
{
	char *p;
	int i, j;
	off_t from, to, eof = pos(ebuf);
	if (here < page) {
		/* Scroll up one logical line or goto physical line. */
		page = row_start(bol(here), here);
	} else if (epage <= here && here < nextline(epage)) {
		/* Scroll down one logical line. */
		page = nextline(page);
	} else if (epage <= here) {
		/* Find top of page from here.  Avoid an unnecessary
		 * screen redraw when the EOF marker is displayed (mid-
		 * screen) by remembering where the previous page frame
		 * started.
		 */
		epage = page;
		for (page = here, i = ROWS - (here == eof); 0 < --i && epage < page; ) {
			page = prevline(page);
		}
		/* When find the top line of the page over shoots the
		 * previous frame, restore the previous frame.  This
		 * avoid an undesired scroll back of one line.
		 */
		if (page <= epage) {
			page = epage;
		}
	} /* Else still within page bounds, update cursor. */
	(void) erase();
	(void) standout();
#ifdef EXT
	(void) printw(
		"%s %ldB %ld%%", filename, eof,
		here * 100 / (eof+(eof <= 0))
	);
	clr_to_eol();
	(void) mvprintw(0, COLS-strlen(mode)-1,"%s%c",mode, chg);
#else /* EXT */
	(void) printw("%s %ldB", filename, here);
	clr_to_eol();
#endif /* EXT */
	if (marker < 0) {
		from = to = marker;
	} else if (here < marker) {
		from = here;
		to = marker;
	} else {
		from = marker;
		to = here;
	}
	for (i = TOP_LINE, j = 0, epage = page; (void) standend(), i < LINES; ) {
		if (here == epage) {
			cur_row = i;
			cur_col = j;
		}
		if (ebuf <= (p = ptr(epage))) {
			break;
		}
#ifdef EXT
		int is_ctrl = iscntrl(*p) && *p != '\t' && *p != '\n';
		if ((from <= epage && epage < to) || is_ctrl) {
			standout();
		}
		/* A multibyte character never stradles the gap,
		 * assumes the gap moves by character, not by byte.
		 * See also nextch() and prevch().
		 */
		int mbl = mblength(*p);
		if (is_ctrl) {
			/* Display control characters as a single byte
			 * highlighted upper case letter (instead of two
			 * byte ^X).
			 */
			(void) mvaddch(i, j, *p+'@');
		} else {
			/* Use addnstr() family that already handles UTF8
			 * instead of add_wch() to avoid all the complexity
			 * of using cchar_t.
			 */
			(void) mvaddnstr(i, j, p, mbl);
		}
		epage += mbl;
#else /* EXT */
		if (from <= epage && epage < to) {
			standout();
		}
		(void) mvaddch(i, j, *p);
		epage++;
#endif /* EXT */
		/* Handle tab expansion ourselves.  Historical
		 * Curses addch() would advance to the next
		 * tabstop (a multiple of 8, eg. 0, 8, 16, ...).
		 * See SUS Curses Issue 7 section 3.4.3.
		 */
		j += charwidth(p, j);
		if (*p == '\n' || COLS <= j) {
			j = 0;
			i++;
		}
	}
	assert(page <= here && here <= epage);
	if (i++ < LINES) {
		(void) standout();
		(void) mvaddstr(i, 0, "^D");
		/* A refresh() side effect is to standend(). */
	}
	(void) move(cur_row, cur_col);
	(void) refresh();
}

void
redraw(void)
{
	(void) clear();
	count = 0;
}

#ifdef EXT
void
left(void)
{
	here = prevch(here);
}
void
right(void)
{
	here = nextch(here);
}
#else /* EXT */
void
left(void)
{
	here -= 0 < here;
}

void
right(void)
{
	here += here < pos(ebuf);
}
#endif /* EXT */

/*
 * Move up one logical line.
 */
void
up(void)
{
	here = col_or_eol(prevline(here), 0, cur_col);
}

/*
 * Move down one logical line.
 */
void
down(void)
{
	here = col_or_eol(nextline(here), 0, cur_col);
}

#ifdef EXT
/*
 * Beginning of physical line.
 */
void
lnbegin(void)
{
	here = bol(here);
}

/*
 * End of physical line.
 */
void
lnend(void)
{
	here = col_or_eol(here, 0, MAX_COLS);
}
#else /* EXT */
#endif /* EXT */

/*
 * Goto column of physical line.
 */
void
column(void)
{
	here = col_or_eol(bol(here), 0, count-1);
	count = 0;
}

void
wleft(void)
{
	/* Move backwards to end of previous word. */
	while (0 < here && isspace(*ptr(here-1))) {
		--here;
	}
	/* Move backwards to start of previous word. */
	while (0 < here && !isspace(*ptr(here-1))) {
		--here;
	}
}

void
pgtop(void)
{
#ifdef EXT
	marks[0] = here;
#else /* EXT */
#endif /* EXT */
	here = page;
}

void
pgbottom(void)
{
#ifdef EXT
	marks[0] = here;
#else /* EXT */
#endif /* EXT */
	here = row_start(bol(epage-1), epage-1);
}

/*
 * Page down logical lines.
 */
void
pgdown(void)
{
	int i;
	/* Maintain cursor row within the next page frame. */
	for (here = epage, i = TOP_LINE; i < cur_row; i++) {
		here = nextline(here);
	}
	/* Maintain cursor column on logical line, if possible. */
	here = col_or_eol(here, 0, cur_col);
	/* Page down advances to the next page or remains as-is because
	 * of a short page at EOF, ie. using short.txt J should not redraw
	 * the screen, just move the cursor to EOF.
	 */
	page = here < pos(ebuf) ? epage : page;
	/* Maintain cursor row within the page by adjusting the frame. */
	for (epage = here; i < LINES; i++) {
		epage = nextline(epage);
	}
}

/*
 * Page up logical lines.
 */
void
pgup(void)
{
	/* Page up N logical lines. */
	for (int i = ROWS; 0 < i--; ) {
		here = prevline(here);
	}
	/* Maintain cursor row within the page by adjusting the frame. */
	for (page = here; TOP_LINE < cur_row--; ) {
		page = prevline(page);
	}
	/* Maintain cursor column on logical line. */
	here = col_or_eol(here, 0, cur_col);
}

void
wright(void)
{
	off_t eof = pos(ebuf);
	/* Move forwards to end of current word. */
	while (here < eof && !isspace(*ptr(here))) {
		++here;
	}
	/* Move forwards to start of next word. */
	while (here < eof && isspace(*ptr(here))) {
		++here;
	}
}

void
lngoto(void)
{
#ifdef EXT
	marks[0] = here;
#else /* EXT */
#endif /* EXT */
	/* Count physcical lines, just as ed, grep, or wc would. */
	off_t eof = pos(ebuf);
	for (here = eof * (count == 0); here < eof && 1 < count; count--) {
		/* Next physical line. */
		here = col_or_eol(here, 0, MAX_COLS);
		here += here < eof;
	}
	/* Set page to eof if beyond page end to force display() to
	 * reframe the page with target line at top.
	 */
	page = eof;
}

#ifdef EXT
int
getsigch(void)
{
	int ch;
	while ((ch = getch()) == CTRL_Z) {
		(void) raise(SIGTSTP);
	}
	return ch;
}
#else /* EXT */
#define getsigch	getch
#endif /* EXT */

void
insert(void)
{
	int ch;
#ifdef EXT
	mode = ins;
	display();
#else /* EXT */
#endif /* EXT */
	movegap(here);
	while ((ch = getsigch()) != CTRL_C && ch != ESC) {
		if (ch == '\b') {
			gap -= buf < gap;
		} else if (gap < egap) {
#ifdef EXT
			/* Using cbreak() then ^V is handled
			 * so ESC ^[ is inserted with ^V^V^[.
			 * With raw() we handle ^V so ESC ^[
			 * is inserted with ^V^[ and we'd have
			 * to handle ^C ourselves.
			 */
			if (ch == CTRL_V) {
				(void) nonl();	/* CR as-is */
				ch = getch();
				(void) nl();	/* CR -> LF */
			}
			for (int n = mblength(ch); 0 < n--; 0 < n && (ch = getch())) {
#else /* EXT */
			{
#endif /* EXT */
				growgap(COLS);
				*gap++ = ch;
				epage++;
			}
		}
		here = pos(egap);
#ifdef EXT
		chg = CHANGED;
#else /* EXT */
#endif /* EXT */
		display();
	}
#ifdef EXT
	mode = cmd;
	adjmarks();
#else /* EXT */
#endif /* EXT */
	/* Not repeatable yet. */
	count = 0;
}

void
yank(void)
{
	off_t mark = marker;
	if (marker < 0) {
		mark = here;
		getcmd(MOTION_CMDS);
	}
	if (mark < here) {
		mark ^= here;
		here ^= mark;
		mark ^= here;
	}
	/* SUS 2018 vi(1) `y` yank sets the cursor on the last column
	 * of the first character of the yanked region; results in yank
	 * forward leaving the cursor as-is, while yank back moves the
	 * cursor back.
	 *
	 * The alternatives are to either always update cursor according
	 * to the motion (better reflects GUI editors, select then copy
	 * or cut, and better visual feedback) or not update cursor at
	 * all (functional but lacks visual feedback ie. did it work).
	 */
	free(scrap);
	movegap(here);
	scrap = strndup(egap, scrap_length = mark-here);
}

void
deld(void)
{
	yank();
	egap += scrap_length;
	here = pos(egap);
#ifdef EXT
	chg = CHANGED;
#else /* EXT */
#endif /* EXT */
	marker = -1;
	adjmarks();
}

void
delx(void)
{
	if (marker < 0) {
		(void) ungetch('l');
	}
	deld();
}

#ifdef EXT
void
delX(void)
{
	if (marker < 0) {
		(void) ungetch('h');
	}
	deld();
}

void
append(void)
{
	right();
	insert();
}
#else /* EXT */
#endif /* EXT */

void
paste(void)
{
	growgap(COLS+(count+(count == 0))*scrap_length);
	if (scrap != NULL) {
		movegap(here);
		(void) memcpy(gap, scrap, scrap_length);
		gap += scrap_length;
		/* SUS 2018 vi(1) `P` paste-before unnamed buffer leaves
		 * the cursor on the last column of the last character.
		 * Instead keep the cursor on the same character before
		 * the paste, which allows for clean repeated pasting,
		 * eg. `PP` or `2P` XX_YY => XXpastepaste_YY, where '_'
		 * is the cursor.
		 */
		here = pos(egap);
		adjmarks();
		/* Force display() to reframe, ie. 1GdGP fails. */
		epage = here+1;
#ifdef EXT
		chg = CHANGED;
#else /* EXT */
#endif /* EXT */
	}
}

#ifdef EXT
void
pastel(void)
{
	right();
	paste();
	/* SUS 2018 vi(1) `p` paste-after unnamed buffer leaves the
	 * cursor on the last column of the last character.  Allows
	 * for common transpose combo `xp`, eg. te_h => th_e.  Also
	 * `pp` or `2p` eg. 1_23 => 12pastepaste_3, where '_' is the
	 * cursor.
	 */
	left();
}

void
setmark(void)
{
	/* ASCII characters ` a..z are the allowed marks. */
	int i = getsigch() - '`';
	if (0 <= i && i < MARKS) {
		marks[i] = here;
	}
}

void
gomark(void)
{
	/* ASCII characters ` a..z are the allowed marks. */
	int i = getsigch() - '`';
	if (0 <= i && i < MARKS) {
		off_t j = marks[0];
		marks[0] = here;
		here = 0 < i ? marks[i] : j;
	}
	count = 0;
}

void
lnmark(void)
{
	int ch = getsigch();
	if (ch == '\'') {
		ch = '`';
	}
	(void) ungetch(ch);
	gomark();
	lnbegin();
}

void
prompt(int ch, const char *str)
{
	(void) echo();
	(void) noraw();
	(void) standout();
	(void) mvaddch(0, 0, ch);
	clr_to_eol();
	/* Prime the input with initial input. */
	ssize_t n = strlen(str);
	assert(n < COLS && COLS <= egap-gap);
	while (0 < n && 0 == ungetch(str[--n])) {
		;
	}
	/* NetBSD 9.3 erase ^H works fine, but not the kill ^U character. */
	(void) mvgetnstr(0, 1, gap, COLS);
	(void) noecho();
	(void) raw();
}

int
filewrite(const char *fn)
{
	int fd;
	movegap(0);
	ssize_t n = 0;
	if (0 < (fd = creat(fn, MODE))) {
		n = write(fd, egap, ebuf-egap);
		(void) close(fd);
	}
	return fd < 0 || n < 0;
}

void
writefile(void)
{
	prompt('>', filename);
	if (*gap != '\0') {
		free(filename);
		filename = strdup(gap);
		if (!filewrite(filename)) {
			chg = NOCHANGE;
		}
	} else {
		(void) beep();
	}
	count = 0;
}

int
fileread(const char *fn)
{
	int fd;
	ssize_t n = 0;
	if (0 < (fd = open(fn, 0))) {
		movegap(here);
		while (0 < (n = read(fd, gap, egap-gap))) {
			gap += n;
			growgap(BUF/2);
		}
		(void) close(fd);
	}
	return (int) n;
}

void
readfile(void)
{
	prompt('<', "");
	if (fileread(gap)) {
		/* ed(1) ? */
		(void) beep();
	} else {
		epage = here+1;
		adjmarks();
	}
	count = 0;
}

/*
 * [countA]![countB]motion command
 *
 * Example format one or more lines of text:
 *
 *	^				Start of line.
 *	!/^$<newline> fmt -w68		Reformat here to next empty line.
 *
 * Can also save a text region from `a to here, eg. !`a tee save.txt
 * If motion is `!` then read-only from command, eg. !! ls
 */
void
bang(void)
{
	ssize_t n;
	pid_t child;
	int child_in[2], child_out[2], ex = 74;
	deld();
	prompt('!', "");
	if (0 == pipe(child_in)) {
		if (0 == pipe(child_out)) {
			if ((child = fork()) >= 0) {
				if (child == 0) {
					/* Redirect standard I/O for the child. */
					if (STDIN_FILENO == dup2(child_in[0], STDIN_FILENO)
					&& STDOUT_FILENO == dup2(child_out[1], STDOUT_FILENO)
					&& STDERR_FILENO == dup2(child_out[1], STDERR_FILENO)) {
						const char *sh, *shell = getenv("SHELL");
						if (NULL == (sh = strrchr(shell, '/'))) {
							sh = shell;
						} else {
							sh++;
						}
						/* Close duplicates and unused. */
						(void) close(child_out[0]);
						(void) close(child_out[1]);
						(void) close(child_in[0]);
						(void) close(child_in[1]);
						/* Pass command(s) through a shell. */
						(void) execl(shell, sh, "-c", gap, NULL);
					}
					/* Use instead of exit() to avoid flushing standard I/O. */
					_exit(127);
				}
				/* Finally write text region to filter and read result. */
				if (scrap != NULL && (scrap_length == 0 || (n = write(child_in[1], scrap, scrap_length)) == scrap_length)) {
					/* Signal EOF write to child. */
					(void) close(child_in[1]);
					/* Wait for the child _before_ reading input. */
					(void) waitpid(child, &ex, 0);
					ex = WIFEXITED(ex) ? WEXITSTATUS(ex) : 127;
					/* Avoid blocking on read() and allow for long or no output. */
					(void) fcntl(child_out[0], F_SETFL, O_NONBLOCK);
					while (0 < (n = read(child_out[0], gap, egap-gap))) {
						gap += n;
						chg = CHANGED;
						growgap(BUF/2);
					}
					here = pos(egap);
					epage = here+1;
					adjmarks();
				}
			}
			(void) close(child_out[0]);
			(void) close(child_out[1]);
		}
		(void) close(child_in[0]);
		(void) close(child_in[1]);
	}
	if (ex != 0) {
		(void) beep();
	}
}

int
cescape(int ch)
{
	for (const char *s = "a\ab\bf\fn\nr\rt\tv\ve\033?\177"; *s != '\0'; s += 2) {
		if (ch == *s) {
			return s[1];
		}
	}
	return ch;
}

void
version(void)
{
	mode = BUILT " " COMMIT;
}

void
quit(void)
{
	mode = NULL;
}

#else /* EXT */
void
writefile(void)
{
	int i;
	movegap(0);
	(void) write(i = creat(filename, MODE), egap, ebuf-egap);
	(void) close(i);
}

void
quit(void)
{
	filename = NULL;
}
#endif /* EXT */

/* In case we're sitting on a previous match, we need to search starting
 * from the end of that match.  Note some special cases:
 *
 * Pattern /^/
 * -----------
 *      BOFtext\n\nterminated\nEOF
 *         ^     ^ ^           ^
 *      BOFtext\n\nnot terminatedEOF
 *         ^     ^ ^
 *
 * Pattern /$/
 * -----------
 *	BOFtext\n\nterminated\nEOF
 *             ^ ^           ^ ^
 *	BOFtext\n\nnot terminatedEOF
 *             ^ ^               ^
 *
 * Pattern /.$/
 * ------------
 *	BOFtext\n\nterminated\nEOF
 *            ^             ^
 *	BOFtext\n\nnot terminatedEOF
 *            ^                 ^
 *
 * Pattern /^$/
 * ------------
 *      BOFtext\n\nterminated\n\nEOF
 *               ^             ^
 *      BOFtext\n\nnot terminatedEOF
 *	         ^               ^
 */
void
next(void)
{
	regmatch_t matches[MATCHES];
	/* Move the gap out of the way in case it sits in the middle
	 * of a potential match and NUL terminate the buffer.
	 */
	assert(gap < egap);
	movegap(pos(ebuf));
#ifdef EXT
	marks[0] = here;
#else /* EXT */
#endif /* EXT */
	*gap = '\0';
	/* REG_NOTBOL allows /^/ to advance to start of next line. */
	if (here+match_length < pos(ebuf) && 0 == regexec(&ere, ptr(here+match_length), MATCHES, matches, REG_NOTBOL)) {
		here += match_length + matches[0].rm_so;
	}
	/* Wrap-around search. */
	else if (0 == regexec(&ere, buf, MATCHES, matches, 0)) {
		here = matches[0].rm_so;
	}
	/* No match after wrap-around. */
	else {
		match_length = 0;
		return;
	}
	match_length = matches[0].rm_eo - matches[0].rm_so + ere_dollar_only;
#ifdef EXT
	if (NULL != replace) {
		movegap(here);
		for (const char *s = replace; *s != '\0'; s++) {
			growgap(COLS);
			if (*s == '$' && isdigit(s[1])) {
				/* Subexpression $0..$9 */
				int i = *++s-'0';
				off_t n = matches[i].rm_eo - matches[i].rm_so;
				(void) memcpy(gap, egap + (matches[i].rm_so - matches[0].rm_so), n);
				gap += n;
				continue;
			} else if (*s == '\\') {
				*gap++ = cescape(*++s);
				continue;
			} else if (*s == '/') {
				/* End replacement string. */
				if (*++s == 'a') {
					/* a = all */
					(void) ungetch('n');
				}
				break;
			}
			*gap++ = *s;
		}
		/* Delete the match. */
		egap += match_length;
		adjmarks();
		/* Replacement string length. */
		match_length = gap-ugap;
		assert(gap < uegap);
	}
#else /* EXT */
#endif /* EXT */
}

void
search(void)
{
#ifdef EXT
	char *t;
	prompt('/', "");
	free(replace);
	/* Find end of pattern. */
	for (t = gap; *t != '\0'; t++) {
		if (*t == '\\') {
			/* Escape next character. */
			t++;
		} else if (*t == '/') {
			/* End of pattern, start of replacement. */
			break;
		}
	}
	if (*t == '/') {
		*t++ = '\0';
		replace = strdup(t);
	}
#else /* EXT */
	(void) echo();
	(void) standout();
	(void) mvaddch(0, 0, '/');
	clr_to_eol();
	assert(COLS <= egap - gap);
	/* NetBSD 9.3 erase ^H works fine, but not the kill ^U character. */
	(void) mvgetnstr(0, 1, gap, egap-gap);
	(void) noecho();
#endif /* EXT */
	regfree(&ere);
	if (regcomp(&ere, gap, REG_EXTENDED|REG_NEWLINE) != 0) {
		/* Something about the pattern is fubar. */
		(void) beep();
	} else {
		/* Kludge to handle repeated /$/ matching. */
		ere_dollar_only = gap[0] == '$' && '\0' == gap[1];
		next();
	}
	count = 0;
}

void
flipcase(void)
{
	char *p = ptr(here);
	if (p < ebuf) {
		/* Skip moving the gap and modify in place. */
		*p = islower(*p) ? toupper(*p) : tolower(*p);
#ifdef EXT
		chg = CHANGED;
#else /* EXT */
#endif /* EXT */
		right();
	}
}

void
anchor(void)
{
	marker = marker < 0 ? here : -1;
}

#ifdef EXT
static char key[] = "hjklbwHJKL^$|G/n`'\006\002~iaxXydPpu!\\mRWQ\003V";

static void (*func[])(void) = {
	/* Motion */
	left, down, up, right, wleft, wright,
	pgtop, pgdown, pgup, pgbottom,
	lnbegin, lnend, column, lngoto,
	search, next, gomark, lnmark,
	pgdown, pgup,
	/* Modify */
	flipcase, insert, append, delx, delX,
	yank, deld, paste, pastel, undo, bang,
	/* Other */
	anchor, setmark, readfile, writefile, quit, quit,
	version, redraw
};
#else /* EXT */
static char key[] = "hjklbwHJKL|G/n~ixydPu\\WQ\003";

static void (*func[])(void) = {
	/* Motion */
	left, down, up, right, wleft, wright,
	pgtop, pgdown, pgup, pgbottom,
	column, lngoto,
	search, next,
	/* Modify */
	flipcase, insert, delx,
	yank, deld, paste, undo,
	/* Other */
	anchor, writefile, quit, quit,
	redraw
};
#endif /* EXT */

void
getcmd(int m)
{
	int j = count, ch;
	for (count = 0; isdigit(ch = getsigch()); ) {
		count = count * 10 + ch - '0';
	}
	/* 2dw = d2w and 2d3w = d6w */
	count = j != 0 && count != 0 ? j*count : count != 0 ? count : j;
	for (j = 0; key[j] != '\0' && ch != key[j]; j++) {
		;
	}
	if (j < m) {
		/* Count always defaults to 1. */
		do (*func[j])(); while (1 < count--);
	}
	count = 0;
}

#ifdef EXT
void
cleanup(void)
{
	/* Most of these are to satisfy Valgrind or saniisers.  The OS
	 * reclaims memory when the program exits, making the need to
	 * free() theoritcally unnecessary.
	 */
	(void) delwin(stdscr);
	(void) endwin();
	free(filename);
	regfree(&ere);
	free(replace);
	free(scrap);
	free(buf);
}
#else /* EXT */
#endif /* EXT */

int
main(int argc, char **argv)
{
	(void) setlocale(LC_ALL, "");
	if (NULL == initscr()) {
		/* Try TERM=ansi-mini which works. */
		return 1;
	}
	(void) noecho();
	(void) raw();
#ifdef EXT
	(void) atexit(cleanup);
	/* Switching between cbreak() and raw() impacts terminal output
	 * which can alter the expected test output files.
	 */
	growgap(BUF);
	filename = strdup(argv[1] == NULL ? "" : *++argv);
	if (fileread(filename)) {
		/* Good grief Charlie Brown! */
		return 2;
	}
	uegap = egap;
	/* Force display() to frame the initial screen. */
	epage = 1;
	while (mode != NULL) {
		display();
		getcmd(ALL_CMDS);
	}
#else /* EXT */
	uegap = egap = ebuf = buf + BUF;
	if (0 < (argc = open(filename = *++argv, 0))) {
		gap += read(argc, buf, ebuf-buf);
		(void) close(argc);
		if (gap < buf || ebuf <= gap) {
			/* Good grief Charlie Brown! */
			return 2;
		}
	}
	/* Force display() to frame the initial screen. */
	epage = 1;
	while (filename != NULL) {
		display();
		getcmd(ALL_CMDS);
	}
	(void) endwin();
#endif /* EXT */
	return 0;
}
