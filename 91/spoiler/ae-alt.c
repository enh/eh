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

#define SLOW_MOVE

#ifndef BUF
# define BUF		(USHRT_MAX)
#endif

#define TABWIDTH	8
#define TABSTOP(col)	(TABWIDTH - ((col) & (TABWIDTH-1)))

static int done;
static int row, col;
static off_t here, page, epage;
static ptrdiff_t gap, egap, ebuf;
static char buf[BUF];
static char *filename;

/*
 * I found a buffer-gap impl. by https://github.com/jarnosz/e, which
 * uses a text buffer base address and offsets for gap, egap, and
 * ebuf, instead of pointers.  This experiment is to see which binary
 * version is smaller depending on the technique used.
 *
 * The original prog.c for the IOCCC conformed to the 1536 bytes of
 * source size rule.  Not yet sure if this version will still conform,
 * though it should for more current versions of the size rule (4096,
 * 2503).
 *
 *	0 <= off_t <= file size	eg. here (the cursor)
 *
 *	0 <= ptrdiff_t <= ebuf	eg. gap (start of the hole)
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
 * Translate a cursor offset into a buffer offset,
 * where the gap size has to be factored in.
 */
ptrdiff_t
ptr(off_t cur)
{
	assert(0 <= cur && cur <= ebuf-(egap-gap));
	return cur + (cur < gap ? 0 : egap-gap);
}

/*
 * Translate a buffer offset into a cursor offset,
 * where the gap size has to be factored out.
 */
off_t
pos(ptrdiff_t off)
{
	assert(0 <= off && off <= ebuf);
	return off - (off < egap ? 0 : egap-gap);
}

void
top(void)
{
	here = 0;
}

void
bottom(void)
{
	epage = here = pos(ebuf);
}

void
quit(void)
{
	done = 1;
}

void
movegap(off_t cur)
{
	assert(0 <= cur && cur <= ebuf-(egap-gap));
#ifdef SLOW_MOVE
	ptrdiff_t p = ptr(cur);
	while (p < gap) {
		buf[--egap] = buf[--gap];
	}
	while (egap < p) {
		buf[gap++] = buf[egap++];
	}
#else /* SLOW_MOVE */
# include <string.h>
	ssize_t len = gap - cur;
	if (len < 0) {
		/* Shift data down, moving gap up to cursor. */
		(void) memcpy(buf+gap, buf+egap, -len);
		egap -= len;
		gap -= len;
	} else {
		/* Shift data up, moving gap down to cursor. */
		gap -= len;
		egap -= len;
		(void) memmove(buf+egap, buf+gap, len);
	}
#endif /* SLOW_MOVE */
	assert(0 <= gap && gap <= egap && egap <= ebuf);
}

off_t
prevline(off_t cur)
{
	while (0 < cur && buf[ptr(--cur)] != '\n') {
		;
	}
	return 0 < cur ? ++cur : 0;
}

off_t
nextline(off_t cur)
{
	off_t n = pos(ebuf);
	while (cur < n && buf[ptr(cur++)] != '\n') {
		;
	}
	return cur < n ? cur : n;
}

off_t
adjust(off_t cur, int column)
{
	ptrdiff_t p;
	for (int i = 0; (p = ptr(cur)) < ebuf && buf[p] != '\n' && i < column; ) {
		i += buf[p] == '\t' ? TABSTOP(i) : 1;
		++cur;
	}
	return cur;
}

void
display(void)
{
	ptrdiff_t p;
	int i, j;
	if (here < page) {
		page = prevline(here);
	}
	if (epage <= here) {
		page = nextline(here);
		i = page == pos(ebuf) ? LINES-2 : LINES;
		while (0 < i--) {
			page = prevline(page-1);
		}
	}
	move(0, 0);
	clrtobot();
	i = j = 0;
	epage = page;
	for (;;) {
		if (here == epage) {
			row = i;
			col = j;
		}
		p = ptr(epage);
		if (LINES <= i || ebuf <= p) {
			break;
		}
		if (buf[p] != '\r') {
			/* Handle tab expansion ourselves.  Differences
			 * between historical Curses and NCurses 2021
			 * WRT tab handling I can't suss just yet.
			 *
			 * Historical Curses addch() would handle tab
			 * expansion as I recall while (current 2021)
			 * NCurses does not by default it seems.
			 */
			if (buf[p] == '\t') {
				j += TABSTOP(j);
				move(i, j);
			} else {
				addch(buf[p]);
				j++;
			}
		}
		if (buf[p] == '\n' || COLS <= j) {
			++i;
			j = 0;
		}
		++epage;
	}
	if (++i < LINES) {
		mvaddstr(i, 0, "~EOF~");
	}
	move(row, col);
	refresh();
}

#ifndef NDEBUG
void
redraw(void)
{
	clear();
	display();
}
#endif /* NDEBUG */

void
left(void)
{
	if (0 < here) {
		--here;
	}
}

void
right(void)
{
	if (here < pos(ebuf)) {
		++here;
	}
}

void
up(void)
{
	here = adjust(prevline(prevline(here)-1), col);
}

void
down(void)
{
	here = adjust(nextline(here), col);
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
	while (0 < here && isspace(buf[ptr(here-1)])) {
		--here;
	}
	/* Move backwards to start of previous word. */
	while (0 < here && !isspace(buf[ptr(here-1)])) {
		--here;
	}
}

void
pgdown(void)
{
	page = here = prevline(epage-1);
	while (0 < row--){
		down();
	}
	epage = pos(ebuf);
}

void
pgup(void)
{
	int i = LINES;
	while (0 < --i) {
		page = prevline(page-1);
		up();
	}
}

void
wright(void)
{
	off_t n = pos(ebuf);
	/* Move forwards to end of current word. */
	while (here < n && !isspace(buf[ptr(here)])) {
		++here;
	}
	/* Move forwards to start of next word. */
	while (here < n && isspace(buf[ptr(here)])) {
		++here;
	}
}

void
insert(void)
{
	int ch;
	movegap(here);
	while ((ch = getch()) != '\e' && ch != '\f') {
		if (ch == '\b') {
			if (0 < gap) {
				--gap;
			}
		} else if (gap < egap) {
			buf[gap++] = ch == '\r' ? '\n' : ch;
		}
		here = pos(egap);
		display();
	}
}

void
delete(void)
{
	movegap(here);
	if (egap < ebuf) {
		here = pos(++egap);
	}
}

void
file(void)
{
	int i;
	movegap(0);
	(void) write(i = creat(filename, MODE), buf+egap, ebuf-egap);
	(void) close(i);
}

void
noop(void)
{
	/* Do nothing. */
}

static char key[] = "hjklHJKL[]tbixWQ"
#ifndef NDEBUG
"R"
#endif /* NDEBUG */
;

static void (*func[])(void) = {
	left, down, up, right,
	wleft, pgdown, pgup, wright,
	lnbegin, lnend, top, bottom,
	insert, delete, file, quit,
#ifndef NDEBUG
	redraw,
#endif /* NDEBUG */
	noop
};

int
main(int argc, char **argv)
{
	int ch, i;
	egap = ebuf = BUF;
	if (argc < 2) {
		return 2;
	}
	initscr();
	raw();
	noecho();
	idlok(stdscr, 1);
	if (0 < (i = open(filename = *++argv, 0))) {
		gap += read(i, buf, ebuf);
		if (gap < 0) {
			gap = 0;
		}
		(void) close(i);
	}
	top();
	while (!done) {
		display();
		ch = getch();
		for (i = 0; key[i] != '\0' && ch != key[i]; i++) {
			;
		}
		(*func[i])();
	}
	endwin();
	return 0;
}
