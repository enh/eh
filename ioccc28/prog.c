#include <curses.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex.h>
#include <locale.h>
int y,x,z,w;
char *f,*s;
char b[BUF],*g = b,*h,*c;
long o,u,v,l,t,n = -1;
regex_t e;
void
T(int);
long 
P(char *s)
{
	return s-b -(s < h ? 0 : h-g);
}
char *
Z(long m)
{
	return b+m +(b+m < g ? 0 : h-g);
}
int
S(int a)
{
	return 1+(a > 193)+(a > 223)+(a > 239);
}
long 
k(long m)
{
	return m +(m < P(c) ?  S(*Z(m)) : 0);
}
long 
Ρ(long m)
{
	while(0 < m &&(192 & *Z(--m)) == 128){
		;
	}
	return m;
}
void
V(long m)
{
	char *p = Z(m);
	while(p < g){
		*--h = *--g;
	}
	while(h < p){
		*g++ = *h++;
	}
}
int
G(char *s,int a)
{
	wchar_t t;
	mbtowc(&t,s,4);
	return t == '\t' ? (8-((a&7))) :(a = wcwidth(t)) < 0 ? 1 : a;
}
long 
O(long m)
{
	while(0 < m && *Z(--m) != '\n'){
		;
	}
	return(m+1) *(0 < m);
}
long 
A(long m,int a,int n)
{
	char *p;
	while(a < n &&(p = Z(m)) < c && *p != '\n'){
		a += G(p,a);
		m = k(m);
	}
	return m;
}
long 
_(long m,long n)
{
	char *p;
	int a = 0;
	long i = m;
	while(m < n &&(p = Z(m = k(m))) < c){
		a += G(p,a);
		if(COLS <= a){
			i = m;
			a = 0;
		}
	}
	return i;
}
long 
M(long m)
{
	long s = O(m);
	long t = _(s,m);
	if(s < t){
		return _(s,t-1);
	}
	return _(O(s-1),s-1);
}
long 
N(long m)
{
	return k(A(m,x,COLS-1));
}
void
C()
{
	printw("%*s",COLS-getcurx(stdscr),"");
}
void
Y()
{
	char *p;
	int i,j;
	long s,t,a = P(c);
	if(o < u){
		u = _(O(o),o);
	}
	else if(v <= o && o < N(v)){
		u = N(u);
	}
	else if(v <= o){
		v = u;
		for(u = o,i = LINES-1 -(o == a); 0 < --i && v < u; ){
			u = M(u);
		}
		if(u <= v){
			u = v;
		}
	}
	erase();
	standout();
	printw("%s %ldB",f,o);
	C();
	if(n < 0){
		s = t = n;
	}
	else if(o < n){
		s = o;
		t = n;
	}
	else {
		s = n;
		t = o;
	}
	for(i = 1,j = 0,v = u;standend(),i < LINES; ){
		if(o == v){
			y = i;
			x = j;
		}
		if(c <=(p = Z(v))){
			break;
		}
		if(s <= v && v < t){
			standout();
		}
		int V = S(*p);
		mvaddnstr(i,j,p,V);
		v += V;
		j += G(p,j);
		if(*p == '\n' || COLS <= j){
			j = 0;
			i++;
		}
	}
	if(i++ < LINES){
		standout();
		mvaddstr(i,0,"^D");
	}
	move(y,x);
	refresh();
}
void
🔥()
{
	clear();
	z = 0;
}
void
H()
{
	o = Ρ(o);
}
void
L()
{
	o = k(o);
}
void
K()
{
	o = A(M(o),0,x);
}
void
J()
{
	o = A(N(o),0,x);
}
void
a()
{
	o = A(O(o),0,z-1);
	z = 0;
}
void
B()
{
	while(0 < o && isspace(*Z(o-1))){
		--o;
	}
	while(0 < o && !isspace(*Z(o-1))){
		--o;
	}
}
void
Ê()
{
	o = u;
}
void
Ë()
{
	o = _(O(v-1),v-1);
}
void
É()
{
	int i;
	for(o = v,i = 1; i < y; i++){
		o = N(o);
	}
	o = A(o,0,x);
	u = o < P(c) ? v : u;
	for(v = o; i < LINES; i++){
		v = N(v);
	}
}
void
È()
{
	for(int i = LINES-1; 0 < i--; ){
		o = M(o);
	}
	for(u = o; 1 < y--; ){
		u = M(u);
	}
	o = A(o,0,x);
}
void
W()
{
	long a = P(c);
	while(o < a && !isspace(*Z(o))){
		++o;
	}
	while(o < a && isspace(*Z(o))){
		++o;
	}
}
void
ө()
{
	long a = P(c);
	for(o = a *(!z); o < a && 1 < z; z--){
		o = A(o,0,999);
		o += o < a;
	}
	u = a;
}
void
I()
{
	int a;
	V(o);
	while((a =getch()) != 3 && a != 27){
		if(a == 8){
			g -= b < g;
		}
		else if(g < h){
			for(int n = S(a); 0 < n--; 0 < n &&(a = getch())){
				*g++ = a;
				v++;
			}
		}
		o = P(h);
		Y();
	}
	z = 0;
}
void
Ô()
{
	long i = n;
	if(n < 0){
		i = o;
		T(14);
	}
	if(i < o){
		i ^= o;
		o ^= i;
		i ^= o;
	}
	free(s);
	V(o);
	s = strndup(h,t = i-o);
	n = -1;
}
void
X()
{
	Ô();
	h += t;
	o = P(h);
}
void
Ó()
{
	if(n < 0){
		ungetch('l');
	}
	X();
}
void
Í()
{
	if(s){
		V(o);
		memcpy(g,s,t);
		g += t;
		o = P(h);
		v = o+1;
	}
}
void
κ()
{
	int i;
	V(0);
	write(i = creat(f,MODE),h,c-h);
	close(i);
}
void
Ф()
{
	f = 0;
}
void
F()
{
	regmatch_t p[1];
	V(P(c));
	*g = 0;
	if(o+l < P(c) && !regexec(&e,Z(o+l),1,p,REG_NOTBOL)){
		o += l + p->rm_so;
	}
	else if(!regexec(&e,b,1,p,0)){
		o = p->rm_so;
	}
	else {
		l = 0;
		return;
	}
	l = p->rm_eo - p->rm_so + w;
}
void
Ñ()
{
	echo();
	standout();
	mvaddch(0,0,'/');
	C();
	mvgetnstr(0,1,g,h-g);
	regfree(&e);
	if(regcomp(&e,g,REG_EXTENDED|REG_NEWLINE)){
		beep();
	}
	else {
		w = g[0] == '$' && !g[1];
		F();
	}
	z = 0;
}
void
Â()
{
	n = n < 0 ? o : -1;
}
char Ќ[] = "hjklbwHJKL|G/nixydP\\WQ\003";
void(*Κ[])(void) = {
	H,J,K,L,B,W,
	Ê,É,È,Ë,
	a,ө,
	Ñ,F,
	I,Ó,
	Ô,X,Í,
	Â,κ,Ф,Ф,
	🔥
};
void
T(int m)
{
	int j = z,a;
	for(z = 0; isdigit(a =getch()); ){
		z = z * 10 + a - '0';
	}
	z = j && z ? j*z : z ? z : j;
	for(j = 0; Ќ[j] && a != Ќ[j]; j++){
		;
	}
	if(j < m){
		do(*Κ[j])(); while(1 < z--);
	}
	z = 0;
}
int
main(int x,char **y)
{
	setlocale(LC_ALL,"");
	if(!initscr()){
		return 1;
	}
	raw();
	h = c = b + BUF;
	if(0 <(x = open(f = *++y,0))){
		g += read(x,b,c-b);
		close(x);
		if(g < b || c <= g){
			return 2;
		}
	}
	v = 1;
	while(f){
		noecho();
		Y();
		T(99);
	}
	endwin();
	return 0;
}
