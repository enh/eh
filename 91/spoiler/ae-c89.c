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

#ifndef BUF
# define BUF		USHRT_MAX
#endif
#ifndef MODE
# define MODE		0600
#endif

#define MAX_COLS	999
#define TABWIDTH	8
#define TABSTOP(col)	(TABWIDTH - ((col) & (TABWIDTH-1)))

static int done, cur_row, cur_col;
static off_t here, page, epage, count;
static char buf[BUF], *filename = "a.txt";
static char *gap = buf, *egap, *ebuf;

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
	char *p = ptr(cur);
	while (p < gap) {
		*--egap = *--gap;
	}
	while (egap < p) {
		*gap++ = *egap++;
	}
	assert(buf <= gap && gap <= egap && egap <= ebuf);
}

/*
 * Return the beginning of previous physical line or BOF.
 */
off_t
prevline(off_t cur)
{
	while (0 < cur && *ptr(--cur) != '\n') {
		;
	}
	/* Return cursor at BOL or BOF. */
	return (cur+1) * (0 < cur);
}

/*
 * Return the column position within the physical line or the EOL,
 * which ever comes first.  Specify a large unrealistic column
 * position, like 999, to simply return the EOL.  A long physical
 * line can wrap into two or more logical lines or screen rows
 * depending on the terminal width given by COLS.
 */
off_t
adjust(off_t cur, int column)
{
	char *p;
	for (int i = 0; (p = ptr(cur)) < ebuf && *p != '\n' && i < column; cur++) {
		i += *p == '\t' ? TABSTOP(i) : 1;
	}
	assert(0 <= cur && cur <= pos(ebuf));
	return cur;
}

/*
 * Return the beginning of next physical line or EOF.
 */
off_t
nextline(off_t cur)
{
	cur = adjust(cur, MAX_COLS);
	/* Return EOF or BOL next line. */
	return cur + (cur < pos(ebuf));
}

void
display(void)
{
	char *p;
	int i, j;
	if (here < page) {
		/* Scroll up one line, page up, or jump up. */
		page = prevline(here);
	} else if (epage <= here && here < nextline(here)) {
		/* Scroll down one line. */
		page = nextline(page);
	} else if (epage < here && here < pos(ebuf)) {
		/* Page down or jump down to line, find top of page. */
		for (page = here, i = LINES; 0 < page && 0 < --i; ) {
			page = prevline(page-1);
		}
	} else if (page == epage && here == pos(ebuf)) {
		/* Jump to EOF. */
		page = prevline(here-1);
	} /* Else still within page bounds, update cursor. */
	move(i = 0, j = 0);
	clrtobot();
	for (epage = page; i < LINES; epage++) {
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
			move(i, j += TABSTOP(j));
		} else {
			addch(*p);
			j++;
		}
		if (*p == '\n' || COLS <= j) {
			++i;
			j = 0;
		}
	}
	if (++i < LINES) {
		mvaddstr(i, 0, "^D");
	}
	move(cur_row, cur_col);
	refresh();
}

void
redraw(void)
{
	clear();
	display();
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

void
up(void)
{
	here = adjust(prevline(prevline(here)-1), cur_col);
}

void
down(void)
{
	here = adjust(nextline(here), cur_col);
}

void
lnbegin(void)
{
	here = prevline(here);
}

void
lnend(void)
{
	here = nextline(here);
	left();
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
pgdown(void)
{
	/* Advance to the next page, then move `here` down. */
	page = here = epage;
	/* Keep the cursor position on the next page, if possible. */
	while (0 < cur_row--){
		down();
	}
	/* Set `epage` beyond `here` so display() will frame
	 * downwards from `page`; `epage` will be updated.
	 */
	epage = pos(ebuf);
}

void
pgup(void)
{
	int i = LINES;
	while (0 < i--) {
		page = prevline(page-1);
		up();
	}
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
	off_t eof = pos(ebuf);
	for (here = eof * (count == 0); here < eof && 1 < count; count--) {
		here = nextline(here);
	}
	/* Set page to eof if beyond page end to force display() to reframe
	 * the page with target line at top.  Otherwise move cursor with the
	 * page.
	 */
	page = here < epage ? page : prevline(eof-1);
	count = 0;
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
		}
		here = pos(egap);
		display();
	}
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
}

void
quit(void)
{
	done = 1;
}

static char key[] = "hjklHJKL[]GixWQ";

static void (*func[])(void) = {
	left, down, up, right,
	wleft, pgdown, pgup, wright,
	lnbegin, lnend, lngoto,
	insert, del, save, quit,
	redraw
};

int
main(int argc, char **argv)
{
	int ch, i;
	initscr();
	cbreak();
	noecho();
	idlok(stdscr, 1);
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
	epage = pos(ebuf);
	while (!done) {
		display();
		ch = getch();
		if (isdigit(ch)) {
			count = count * 10 + ch - '0';
		} else {
			for (i = 0; key[i] != '\0' && ch != key[i]; i++) {
				;
			}
			(*func[i])();
		}
	}
	endwin();
	return 0;
}
