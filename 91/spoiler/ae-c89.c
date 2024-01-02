/*
 * Anthony's Editor
 *
 * Public Domain 1991, 2023 by Anthony Howe.  All rights released.
 */

#include <ctype.h>
#include <assert.h>
#include <curses.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <regex.h>

#ifndef BUF
# define BUF		USHRT_MAX
#endif
#ifndef MODE
# define MODE		0600
#endif

#define MAX_COLS	999
#define TABWIDTH	8
#define TABSTOP(col)	(TABWIDTH - ((col) & (TABWIDTH-1)))

#define STANDOUT_EOF	1
#define TOP_LINE	1
#define ROWS		(LINES-TOP_LINE)

static int done, cur_row, cur_col, count, ere_dollar_only, match_length;
static char buf[BUF], *filename = "a.txt";
static char *gap = buf, *egap, *ebuf;
static off_t here, page, epage;
static regex_t ere;

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
movegap(off_t cur)
{
	assert(0 <= cur && cur <= pos(ebuf));
#ifdef FAST_MOVE
# include <string.h>
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
 * Return the beginning of next physical line or EOF.
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
	for (int i = getcurx(stdscr); i < COLS; i++) {
		(void) addch(' ');
	}
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
		/* Handle tab expansion ourselves.  Differences
		 * between historical Curses and NCurses 2021
		 * WRT tab handling I can't suss just yet.
		 *
		 * Historical Curses addch() would handle tab
		 * expansion as I recall while (current 2021)
		 * NCurses does not by default it seems.
		 */
		if (*p == '\t') {
			(void) move(i, j += TABSTOP(j));
		} else {
			(void) addch(*p);
			j++;
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
	here = page;
}

void
pgbottom(void)
{
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
	off_t n = pos(ebuf);
	/* Move forwards to end of current word. */
	while (here < n && !isspace(*ptr(here))) {
		++here;
	}
	/* Move forwards to start of next word. */
	while (here < n && isspace(*ptr(here))) {
		++here;
	}
}

void
lngoto(void)
{
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
	while ((ch = getch()) != '\e' && ch != '\f') {
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
	count = 0;
}

void
del(void)
{
	movegap(here);
	if (egap < ebuf) {
		here = pos(++egap);
	}
}

void
save(void)
{
	int i;
	movegap(0);
	(void) write(i = creat(filename, MODE), egap, ebuf-egap);
	(void) close(i);
	count = 0;
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
	*gap = '\0';
	/* REG_NOTBOL allows /^/ to advance to start of next line. */
	if (here+match_length < pos(ebuf) && 0 == regexec(&ere, ptr(here+match_length), 1, matches, REG_NOTBOL)) {
		here += match_length + matches[0].rm_so;
	}
	/* Wrap-around search. */
	else if (0 == regexec(&ere, ptr(0), 1, matches, 0)) {
		here = matches[0].rm_so;
	}
	/* No match after wrap-around. */
	else {
		match_length = 0;
		(void) beep();
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
	(void) mvgetnstr(0, 1, gap, egap-gap);
	(void) standend();
	(void) noecho();
	regfree(&ere);
	if (regcomp(&ere, gap, REG_EXTENDED|REG_NEWLINE) != 0) {
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
	/* Skip moving the gap and modify in place. */
	*p = islower(*p) ? toupper(*p) : tolower(*p);
	right();
}

void
quit(void)
{
	done = 1;
}

static char key[] = "hjklbwHJKL[]Gix~W/nQ";

static void (*func[])(void) = {
	left, down, up, right, wleft, wright,
	pgtop, pgdown, pgup, pgbottom,
	lnbegin, lnend, lngoto,
	insert, del, flipcase, save,
	search, next, quit,
	redraw
};

int
main(int argc, char **argv)
{
	int ch, i;
	if (NULL == initscr()) {
		/* Try TERM=ansi-mini which works. */
		return 1;
	}
	/* We could use raw(), but cbreak() still allows for signals, like
	 * CTRL+Z (suspend) and CTRL+C (interrupt) (I don't read manuals
	 * and don't know how to quit).
	 */
	(void) cbreak();
	(void) noecho();
//	/* Curses draw optimisations can use scrolling. */
//	(void) idlok(stdscr, 1);
	egap = ebuf = buf + BUF;
	if (1 < argc) {
		filename = *++argv;
	}
	if (0 < (i = open(filename, 0))) {
		gap += read(i, buf, BUF);
		(void) close(i);
		if (gap < buf) {
			/* Good grief Charlie Brown! */
			return 1;
		}
	}
	/* Force display() to frame the initial screen. */
	epage = 1;
	while (!done) {
		display();
		ch = getch();
		if (isdigit(ch)) {
			count = count * 10 + ch - '0';
		} else {
			for (i = 0; key[i] != '\0' && ch != key[i]; i++) {
				;
			}
			do (*func[i])(); while (0 < --count);
			count = 0;
		}
	}
	(void) endwin();
	return 0;
}
