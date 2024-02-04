/*
 * Anthony's Editor
 *
 * Public Domain 1991, 2024 by Anthony Howe.  All rights released.
 */

#include <ctype.h>
#include <assert.h>
#include <curses.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#ifndef BUF
# define BUF		(128*1024)
#endif
#ifndef MODE
# define MODE		0600
#endif

#define MAX_COLS	999
#define TABWIDTH	8
#define TABSTOP(col)	(TABWIDTH - ((col) & (TABWIDTH-1)))

#define TOP_LINE	1
#define ROWS		(LINES-TOP_LINE)

#ifdef ALT
#define MOTION_CMDS	16
#else
#define MOTION_CMDS	15
#endif /* ALT */
#define ALL_CMDS	99

static int cur_row, cur_col, count, ere_dollar_only;
static char buf[BUF], *filename, *scrap;
static char *gap = buf, *egap, *ebuf, *ugap = buf, *uegap;
static off_t here, page, epage, uhere, match_length, scrap_length, marks[27];
static regex_t ere;

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
pos(char *s)
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

void
setundo(void)
{
	ugap = gap;
	uegap = egap;
	uhere = here;
}

void
undo(void)
{
	char *m = ugap, *n = uegap;
	off_t p = uhere;
	setundo();
	here = p;
	egap = n;
	gap = m;
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
		col += *p == '\t' ? TABSTOP(col) : 1;
		cur++;
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
	int col = 0;
	off_t mark = cur;
	assert(/* 0 <= cur && cur <= offset &&*/ offset <= pos(ebuf));
	while (cur < offset) {
		cur++;
		col += *ptr(cur) == '\t' ? TABSTOP(col) : 1;
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
	cur = col_or_eol(cur, cur_col, COLS-1);
	return cur + (cur < pos(ebuf));
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
		for (page = here, i = ROWS - (here == pos(ebuf)); 0 < --i && epage < page; ) {
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
	(void) standout();
	(void) mvprintw(0, 0, "%s %ldB", filename, (long) pos(ebuf));
	clr_to_eol();
	(void) standend();
	(void) clrtobot();
	for (i = TOP_LINE, j = 0, epage = page; i < LINES; epage++) {
		if (here == epage) {
			cur_row = i;
			cur_col = j;
		}
		if (ebuf <= (p = ptr(epage))) {
			break;
		}
		if (*p != '\r') {
			/* Handle tab expansion ourselves.  Historical
			 * Curses addch() would advance to the next
			 * tabstop (a multiple of 8, eg. 0, 8, 16, ...).
			 * See SUS Curses Issue 7 section 3.4.3.
			 *
			 * Note that the behaviour of LF will typically
			 * blank-out or truncated the current line, which
			 * means "text\r\n" would result in "text" being
			 * erased, eg. CR move to column 0, LF clear to
			 * end line and move to column 0 of next line.
			 *
			 * CR and other non-spacing control characters
			 * still count as a byte when framing and placing
			 * the cursor, but are not visible.
			 */
			(void) mvaddch(i, j, *p);
			j += *p == '\t' ? TABSTOP(j) : 1;
		}
		if (*p == '\n' || COLS <= j) {
			j = 0;
			i++;
		}
	}
	assert(page <= here && here <= epage);
	if (i++ < LINES) {
		(void) standout();
		(void) mvaddstr(i, 0, "^D");
		(void) standend();
	}
	(void) move(cur_row, cur_col);
	(void) refresh();
}

void
redraw(void)
{
	(void) clear();
//	count = 0;
}

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

#ifdef ALT
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
#else
/*
 * Goto column of physical line.
 */
void
column(void)
{
	here = col_or_eol(bol(here), 0, count-1);
	count = 0;
}
#endif /* ALT */

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
	marks[0] = here;
	here = page;
}

void
pgbottom(void)
{
	marks[0] = here;
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
	marks[0] = here;
	/* Count physcical lines, just as ed, grep, or wc would. */
	off_t eof = pos(ebuf);
	for (here = eof * (count == 0); here < eof && 1 < count; count--) {
		/* Next physical line. */
		here = col_or_eol(here, 0, MAX_COLS);
		here += here < eof;
	}
// 	/* Set page to eof if beyond page end to force display() to
// 	 * reframe the page with target line at top.  Otherwise move
// 	 * the cursor within the page.
// 	 */
// 	page = here <= epage ? page : eof;
	/* Set page to eof if beyond page end to force display() to
	 * reframe the page with target line at top.
	 */
	page = eof;
}

void
insert(void)
{
	int ch;
	movegap(here);
	while ((ch = getch()) != ' '-5/* && ch != '\f'*/) {
		if (ch == '\b') {
			gap -= buf < gap;
		} else if (gap < egap) {
			*gap++ = ch;
			epage++;
		}
		here = pos(egap);
		display();
	}
	/* Not repeatable yet. */
//	count = 0;
}

/*
 * Similar to `dl` command, eg.
 *
 *	ungetc('l', stdin);
 *	deld();
 */
void
delx(void)
{
	movegap(here);
	egap += egap < ebuf;
}

void
yank(void)
{
	off_t mark = here;
	getcmd(MOTION_CMDS);
	if (mark < here) {
		mark ^= here;
		here ^= mark;
		mark ^= here;
	}
	free(scrap);
	movegap(here);
	scrap = strndup(egap, scrap_length = mark-here);
}

void
deld(void)
{
	yank();
	egap += scrap_length;
}

void
paste(void)
{
	if (scrap != NULL && scrap_length <= egap-gap) {
		movegap(here);
		(void) memcpy(gap, scrap, scrap_length);
		gap += scrap_length;
		/* Force display() to reframe, ie. 1GdGP fails. */
		epage = here+1;
	}
}

void
setmark(void)
{
	/* ASCII characters ` a..z are the allowed marks. */
	int i = getch() - '`';
	if (0 <= i && i < 27) {
		marks[i] = here;
	}
}

void
gomark(void)
{
	/* ASCII characters ` a..z are the allowed marks. */
	int i = getch() - '`';
	if (0 <= i && i < 27) {
		off_t j = marks[0];
		marks[0] = here;
		here = 0 < i ? marks[i] : j;
	}
}

void
save(void)
{
	int i;
	movegap(0);
	(void) write(i = creat(filename, MODE), egap, ebuf-egap);
	(void) close(i);
//	count = 0;
}

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
	regmatch_t matches[1];
	/* Move the gap out of the way in case it sits in the middle
	 * of a potential match and NUL terminate the buffer.
	 */
	assert(0 < egap - gap);
	movegap(pos(ebuf));
	marks[0] = here;
	*gap = '\0';
	/* REG_NOTBOL allows /^/ to advance to start of next line. */
	if (here+match_length < pos(ebuf) && 0 == regexec(&ere, ptr(here+match_length), 1, matches, REG_NOTBOL)) {
		here += match_length + matches[0].rm_so;
	}
	/* Wrap-around search. */
	else if (0 == regexec(&ere, buf, 1, matches, 0)) {
		here = matches[0].rm_so;
	}
	/* No match after wrap-around. */
	else {
		match_length = 0;
		return;
	}
	match_length = matches[0].rm_eo - matches[0].rm_so + ere_dollar_only;
}

void
search(void)
{
	(void) echo();
	(void) standout();
	(void) mvaddch(0, 0, '/');
	clr_to_eol();
	assert(COLS <= egap - gap);
	/* Erase (^H) works fine, but not the kill (^U) character. */
	(void) mvgetnstr(0, 1, gap, egap-gap);
	(void) standend();
	(void) noecho();
	regfree(&ere);
	if (regcomp(&ere, gap, REG_EXTENDED|REG_NEWLINE) != 0) {
		/* Something about the pattern is fubar. */
		(void) beep();
	} else {
		/* Kludge to handle repeated /$/ matching. */
		ere_dollar_only = gap[0] == '$' && '\0' == gap[1];
		next();
	}
	/* Does not make sense to repeat. */
//	count = 0;
}

void
flipcase(void)
{
	char *p = ptr(here);
	/* Skip moving the gap and modify in place. */
	*p = islower(*p) ? toupper(*p) : tolower(*p);
	right();
}

void
quit(void)
{
	filename = NULL;
}

#ifdef ALT
static char key[] = "hjklbwHJKL^$G/n`~ixydPumWQ";
#else
static char key[] = "hjklbwHJKL|G/n`~ixydPumWQ";
#endif /* ALT */

static void (*func[])(void) = {
	/* Motion */
	left, down, up, right, wleft, wright,
	pgtop, pgdown, pgup, pgbottom,
#ifdef ALT
	lnbegin, lnend, lngoto,
#else
	column, lngoto,
#endif /* ALT */
	search, next, gomark,
	/* Modify */
	flipcase, insert, delx, yank, deld, paste, undo,
	/* Other */
	setmark, save, quit,
	redraw
};

void
getcmd(int m)
{
	int j = count, ch;
	for (count = 0; isdigit(ch = getch()); ) {
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
		count = 0;
	}
}

int
main(int argc, char **argv)
{
	int i;
	if (NULL == initscr()) {
		/* Try TERM=ansi-mini which works. */
		return 1;
	}
	/* We could use raw(), but cbreak() still allows for signals, like
	 * CTRL+Z (suspend) and CTRL+C (interrupt) (for those who don't
	 * read manuals and don't know how to quit).
	 */
	(void) cbreak();
	(void) noecho();
	uegap = egap = ebuf = buf + BUF;
	if (0 < (i = open(filename = *++argv, 0))) {
		gap += read(i, buf, ebuf-buf);
		(void) close(i);
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
	return 0;
}
