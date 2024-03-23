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
#include <signal.h>
#include <regex.h>
#include <sys/wait.h>

#ifndef BUF
# define BUF		(128*1024)
#endif
#ifndef MODE
# define MODE		0600
#endif

#define ALT
#define EXT

#define MARKS		27
#define MAX_COLS	999
#define TABWIDTH	8
#define TABSTOP(col)	(TABWIDTH - ((col) & (TABWIDTH-1)))

#define TOP_LINE	1
#define ROWS		(LINES-TOP_LINE)

#define MOTION_CMDS	18
#define ALL_CMDS	99

static int cur_row, cur_col, count, ere_dollar_only;
static char *filename, *scrap, *replace;
static char *buf, *gap, *egap, *ebuf, *ugap, *uegap;
static char ins[] = "INS", cmd[] = "   ", *mode = cmd;
static off_t here, page, epage, uhere, match_length, scrap_length, marks[MARKS];
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
adjmarks(void)
{
	off_t n = /* insert, paste */(gap-ugap) - /* delete */(egap-uegap);
	for (int i = 0; i < MARKS; i++) {
		if (here < marks[i]) {
			marks[i] += n;
		}
	}
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

void
growgap(size_t min)
{
	char *xbuf;
	/* Remember current gap location. */
	assert(buf <= gap && gap <= egap && egap <= ebuf);
	/* The gap is used for field input and should be at least as
	 * large as the screen width.
	 */
	if ((size_t)(egap-gap) < min) {
		off_t xhere = pos(egap);
		ptrdiff_t buflen = ebuf-buf;
		/* Append the new space to the end of the gap. */
		movegap(pos(ebuf));
		if (NULL == (xbuf = realloc(buf, buflen+BUF))) {
			/* Bugger.  Don't exit, allow user to write file. */
			(void) beep();
			return;
		}
		egap = ebuf = xbuf+buflen+BUF;
		gap = xbuf+(gap-buf);
		buf = xbuf;
		/* Restore gap's previous location. */
		movegap(xhere);
	}
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
	(void) mvaddstr(0, COLS-3, mode);
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
#ifdef ALT
		/* Handle tab expansion ourselves.  Historical
		 * Curses addch() would advance to the next
		 * tabstop (a multiple of 8, eg. 0, 8, 16, ...).
		 * See SUS Curses Issue 7 section 3.4.3.
		 *
		 * Display control characters as a single byte
		 * highlighted upper case letter (instead of two
		 * byte ^X).
		 */
		(void) mvaddch(i, j, isprint(*p) || *p == '\t' || *p == '\n' ? *p : A_STANDOUT|(*p+'@'));
		j += *p == '\t' ? TABSTOP(j) : 1;
#else
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
#endif /* ALT */
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
	count = 0;
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

int
getsigch(void)
{
	int ch;
	while ((ch = getch()) == '\032' /* ^Z */) {
		(void) raise(SIGTSTP);
	}
	return ch;
}

void
insert(void)
{
	int ch;
	mode = ins;
	display();
	movegap(here);
	while ((ch = getsigch()) != '\033' /* ^[ */ && ch != '\003' /* ^C */) {
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
			if (ch == '\026' /* ^V */) {
				(void) nonl();	/* CR as-is */
				ch = getch();
				(void) nl();	/* CR -> LF */
			}
#else
#endif /* EXT */
			growgap(COLS);
			*gap++ = ch;
			epage++;
		}
		here = pos(egap);
		display();
	}
	mode = cmd;
	adjmarks();
	/* Not repeatable yet. */
	count = 0;
}

void
append(void)
{
	right();
	insert();
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
	adjmarks();
}

void
delx(void)
{
	(void) ungetc('l', stdin);
	deld();
}

void
delX(void)
{
	(void) ungetc('h', stdin);
	deld();
}

void
paste(void)
{
	growgap(COLS+(count+(count == 0))*scrap_length);
	if (scrap != NULL) {
		movegap(here);
		do {
			(void) memcpy(gap, scrap, scrap_length);
			gap += scrap_length;
		} while (1 < count--);
		/* SUS 2018 vi(1) `P` paste-before unnamed buffer leaves
		 * the cursor on the last column of the last character.
		 * Instead keep the cursor on the same character before
		 * the paste, which allows for clean repeated pasting,
		 * eg. `PP` or `2P` XX_YY => XXpastepaste_YY, where '_'
		 * is the cursor.
		 */
		here = pos(egap);
		/* Force display() to reframe, ie. 1GdGP fails. */
		epage = here+1;
	}
}

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
	(void) ungetc(ch, stdin);
	gomark();
	lnbegin();
}

void
writefile(void)
{
	int i;
	movegap(0);
	(void) write(i = creat(filename, MODE), egap, ebuf-egap);
	(void) close(i);
	count = 0;
}

void
prompt(int ch)
{
	(void) echo();
	(void) standout();
	(void) mvaddch(0, 0, ch);
	clr_to_eol();
	assert(COLS <= egap - gap);
	/* NetBSD 9.3 erase ^H works fine, but not the kill ^U character. */
	(void) mvgetnstr(0, 1, gap, egap-gap);
	(void) standend();
	(void) noecho();
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
	prompt('<');
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
	int child, pipein[2], pipeout[2], ex = 74;
	deld();
	prompt('!');
	if (*gap == '\0' || pipe(pipein)) {
		goto error0;
	}
	if (pipe(pipeout)) {
		goto error1;
	}
	if ((child = fork()) < 0) {
		goto error2;
	}
	if (child == 0) {
		/* Redirect standard I/O for the child. */
		if (STDIN_FILENO == dup2(pipeout[0], STDIN_FILENO)
		&& STDOUT_FILENO == dup2(pipein[1], STDOUT_FILENO)
		&& STDERR_FILENO == dup2(pipein[1], STDERR_FILENO)) {
			/* The Child, close the ends of the pipes not used. */
			(void) close(pipeout[1]);
			(void) close(pipein[0]);
			ex = system(gap+(*gap == '!')) < 0;
		}
		exit(ex);
	}
	/* Finally write text region to filter and read result. */
	if (scrap != NULL && 0 <= write(pipeout[1], scrap, scrap_length)) {
		/* Signal EOF write to child, else hang on read(). */
		(void) close(pipeout[1]);
		ssize_t n = read(pipein[0], gap, egap-gap);
		if (0 <= n && n < egap-gap) {
			gap += n;
			adjmarks();
			epage = here+1;
		} else {
			/* ed(1) ? */
			beep();
		}
		/* Get the child exit code; probably redundant. */
		(void) waitpid(child, &ex, 0);
	}
error2:
	(void) close(pipeout[0]);
	(void) close(pipeout[1]);
error1:
	(void) close(pipein[0]);
	(void) close(pipein[1]);
error0:
	;
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
	regmatch_t matches[10];
	/* Move the gap out of the way in case it sits in the middle
	 * of a potential match and NUL terminate the buffer.
	 */
	assert(gap < egap);
	movegap(pos(ebuf));
	marks[0] = here;
	*gap = '\0';
	/* REG_NOTBOL allows /^/ to advance to start of next line. */
	if (here+match_length < pos(ebuf) && 0 == regexec(&ere, ptr(here+match_length), 10, matches, REG_NOTBOL)) {
		here += match_length + matches[0].rm_so;
	}
	/* Wrap-around search. */
	else if (0 == regexec(&ere, buf, 10, matches, 0)) {
		here = matches[0].rm_so;
	}
	/* No match after wrap-around. */
	else {
		match_length = 0;
		return;
	}
	match_length = matches[0].rm_eo - matches[0].rm_so + ere_dollar_only;
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
					(void) ungetc('n', stdin);
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
}

void
search(void)
{
	char *t;
	prompt('/');
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
	filename = NULL;
}

static char key[] = "hjklbwHJKL^$|G/n`'~iaxXydPpu!mRWQ\003";

static void (*func[])(void) = {
	/* Motion */
	left, down, up, right, wleft, wright,
	pgtop, pgdown, pgup, pgbottom,
	lnbegin, lnend, column, lngoto,
	search, next, gomark, lnmark,
	/* Modify */
	flipcase, insert, append, delx, delX,
	yank, deld, paste, pastel, undo, bang,
	/* Other */
	setmark, readfile, writefile, quit, quit,
	redraw
};

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

void
cleanup(void)
{
	(void) endwin();
	regfree(&ere);
	free(replace);
	free(scrap);
}

int
main(int argc, char **argv)
{
	if (NULL == initscr()) {
		/* Try TERM=ansi-mini which works. */
		return 1;
	}
	(void) atexit(cleanup);
	/* Switching between cbreak() and raw() impacts terminal output
	 * which can alter the expected test output files.
	 */
#ifdef ALT
	(void) raw();
#else
	(void) cbreak();
#endif /* ALT */
	(void) noecho();
	growgap(BUF);
	if (fileread(filename = *++argv)) {
		/* Good grief Charlie Brown! */
		return 2;
	}
	uegap = egap;
	/* Force display() to frame the initial screen. */
	epage = 1;
	while (filename != NULL) {
		display();
		getcmd(ALL_CMDS);
	}
	return 0;
}
