
/*
	edx (EDitor for X), (C) 2003, Terry Loveall
	released into the public domain.
	Based upon the original work ee.c of Yijun Ding, copyright 1991 which is
	in the public domain.
	This program comes with no warranties or binaries. Use at your own risk.
*/

#define VOID_LINK (unsigned int)-1

#ifdef GREEK
/* Greek letters */
static char *greek_letter[26] = {
"alpha", "!beta", "Chi", "Delta", "!epsilon", "!Phi",
"Gamma", "eta", "iota", "Psi", "kappa", "Lambda", "mu", "nu",
"omega", "Pi", "!Theta", "!rho", "!Sigma", "tau", "Upsilon", NULL,
"Omega", "Xi", "Psi", "zeta"
};
int var_greek = 0;                         /* variant of Greek letters */
#endif

int x=0, screen_width;		/* screen position 1 <= x <= screen_width */
int y=1, screen_height; 	/* screen size 0 <= y <= screen_height */
int eolx;					/* coord of eol */
int update_scr = 1; 		/* do screen updates flag */
int first_time = 1;			/* first time flag for stdin use */

char *bstart, *bend; 		/* marked block start, end and char pointer */
char *binding[26];

#define YTOP	0		/* first line */

char  bbuf[NLEN]; 			  /* temp file name */
char  sbuf[NLEN], rbuf[NLEN]; /* search buffer, replace buffer */
char  *s;					  /* pointer to search string */
int   rlen, slen;			  /* replace and search lengths */

unsigned blen;				  /* length of text in block buffer */
char *cur_pos;
char *line_start;				/* current pos, start of current line */
char *screen_start;				/* global screen start */
unsigned int xlo;				/* x offset for left display edge */
char *last_pos;					/* last last position visited */
char *old_pos;					/* last position visited */

int x_offset;					/* offset of xtru from line_start */
int xtru = 0, ytru = 0; 		/* file position */
int ytot = 0;					/* 0 <= ytru <= ytot */

int y1, y2; 				  /* 1st, 2nd line of window */
int tabsize=4;				  /* tab size */
int doCtrlC = 0;				/* decode next char from ^C function */
int doCtrlK = 0;				/* decode next char from ^K function */
int doCtrlQ = 0;				/* decode next char from ^Q function */
int doCtrlX = 0;				/* decode next char from ^X function */
int doEscap = 0;				/* decode next char from Esc function */
int help_done = 0;
int literal = 0;

int col;						/* width of dialog preset (if any) */
int diastart;					/* start of dialog input field */
int first;						/* first dialog char flag, 0=true */
int dblen;						/* current dialog buffer size */
char *diabuf;					/* dialog buffer pointer */
void (*dialogCB) ();			/* callback pointer */

static char lbuf[16];			/* goto line number buffer */
static char cbuf[16];			/* goto column number buffer */
static char wbuf[8]="78";		/* reformat block right margin */
static char twbuf[8]="4";		/* tab width buffer */

static char mbuf[NLEN];			/* macro record buffer */
static char *pmbuf=mbuf;		/* record pointer for macro buffer */
struct stat ofstat;				/* edit file stat structure */

FILE  *fi, *fb;				/* file handles for input, block */
static int fo;				/* file handle for output */
static char fbuf[NLEN];			/* current file name buffer */
#ifndef TERMINAL
static char newbuf[NLEN];		/* new file name buffer */
#endif /* TERMINAL */

char	flag[WIN+1]="\0\0\0C\0\0N\0\0";	/* options flag presets */
char	fsym[]="MFOCTBNRAW";	/* Modified,Fill,Overwrite,Case,Tab,Block marked,autoiNdent,Rec,All,Word */

/* word delimiter chars */
char wdelims[] = "\t ,;+*=^&|?:`()[]{}<>\"\'";
char mdelims[] = "\t ,;+-*=^&|?`()[]{}<>\"\'";
char sdelims[] = "\t >]})";
char s1delims[] = "\t ";

/* current window struct */
typedef struct {
	char name[NLEN];
	int jump;
} MWIN; 	/* my window structure */

MWIN  ewin;  /* current window */

char  *file_end;		/* end of file */
char  *mk;				/* mark */
char  *last_mk = NULL;	/* prev mk position */

#ifdef STATICBUF
/* static buffer allocation scheme */
char  bb[BMAX];			/* block buffer */
char  edbuf[AMAX];		/* edit buffer */
char  unbuf[AMAX];		/* undo stack */

/* undo record struct */
typedef struct {
	void *link;
	char *pos;
	int length;
	char str[1];
} U_REC;

#else

/* dynamic buffer allocation scheme */
#define UMAX 0x2000		/* undo level */
unsigned int amax, bmax, umax;	/* main buffer, block size, undo level */
char  *bb;				/* block buffer */
char  *edbuf;			/* edit buffer */
void  *unbuf;			/* undo stack */

/* undo record struct */
typedef struct {
	unsigned int link;
	int pos;
	int length;
	char str[0];
} U_REC;
#endif /* STATICBUF */

int undo_wrap = 0;		/* undo buffer wrap flag */
U_REC* undop;			/* active undo/redo pointer */
U_REC* undosp;			/* top of undo stack pointer */
int in_undo;			/* dont reset undop to undosp if true */
int undone=1; 			/* have undone all */

/* function prototypes */

int get_tru(char* pos);
char* get_cur();
void init_undo();
U_REC* new_undo(char *pos, int len);
void u_del(char *pos, int len);
void u_ins(char *pos, int len);
void ytot_ins();
void undo();
void redo();
void sys_exit(int code);
void bell();
void move_to(int newx, int newy);
void getx();
void cursor_up(), cursor_down(), cursor_left(), cursor_right();
void cursor_pageup();
void cursor_pagedown();
void word_left();
void word_right();
void word_mark();
void word_marknopen();
void drstr(char* disp, int i);
void show_rest(int len, char *s);
void show_scr(int fr, int to);
void scroll_up();
void scroll_down();
void show_sdn(int line);
void show_flag(int x, int g);
void dialog(int key);
void show_note(char *prp);
int show_gets(char *prp, char *buf, int blen, void *cb);
void options(int key);
void show_pos();
void show_top(), show_help(), show_mode();
void top();
char *file_new();
void file_read();
char *file_ltab(char *s);
int  file_write(int fd, char *s, char *e);
int  file_fout();
void do_save();
#ifndef TERMINAL
void set_title();
int SYSTEM (char *cmd);
void gorun();
void new_edit(char *path, char *fn);
void newedit(char *fn);
void mterm();
#endif /* TERMINAL */
void do_cd();
void chgdir();
void do_open();
void file_save(int f_all, int f_win);
void newfile();
void file_resize(char *s, char *d);
void goto_x(int xx), goto_y(int yy);
void goto_ptr(char *s);
void goline();
void goto_last();
void goto_line(), goto_col();
void gocol();
void goto_col();
void twist_pos();
int  str_cmp(char *s);
void gofind();
char *goto_find(char *s, int  back);
void get_mk(char* dbuf);
char *goto_search(int	back);
void dorepl();
void doSAR();
void goto_replace(int all);
void gettab();
void tab_size();
void getmarg();
void window_size();
void mark_off();
void block_put(), block_get(), block_mark();
void remove_block();
void block_remove_update();
void chg_case(int upper);
void transpose();
void block_copy(int delete);
void block_paste(), block_write(), block_line();
void doblockr();
void doblockw();
void block_read();
int  block_fill();
void find_match();
void block_format();
void key_return(), key_deleol(char *s), key_delete();
void key_backspace(), key_delword(int eol);
void key_tab(int tabi);
void key_normal(int key);
void key_macros(int record);
#ifdef GREEK
void l(int key);
#endif
void ctrlc_key(int key);
void ctrlk_key(int key);
void ctrlq_key(int key);
void key_control(int key);
#ifdef TERMINAL
void main_meta(int key);
#else
void key_alt(int key);
void key_escape(int key);
void key_func(int key);
#endif /* TERMINAL */
void scr_update();
void main_exec(int key);

/* code */

/*
void prrec(U_REC* undp, char indicator)
{
		char str[NLEN];

		if(undp == NULL) return;

		memset(str,0,NLEN);
		memcpy(str, undp->str, undp->length >= 0 ?  undp->length: 0 - undp->length);
		printf("%c:p=%x, l=%x, pos=%x, ln=%x, str=%s.\n",
				indicator,
				(unsigned int)undp,
				(unsigned int)undp->link + unbuf,
				(unsigned int)(undp->pos + edbuf),
				undp->length,
				str);
}
*/

/* get true x co-ord for char at 'pos' */
int get_tru(char* pos)
{
	char* tmpp;
	int i = 1;

	tmpp = line_start + strlen(line_start+1)+1;
		if(line_start > pos || tmpp < pos) {
#ifdef DEBUG
			printf("Pos=%x, ls=%x, le=%x.\n",
					(unsigned int)pos,
					(unsigned int)line_start,
					(unsigned int)tmpp);
#else
			printf("get_tru\n");
			bell();
#endif /* DEBUG */
			return 0;
		}
		for(tmpp = line_start+1; tmpp < pos; tmpp++){
			if(*tmpp != '\t') {i++;}
			else {
				i = i + ((tabsize) - ((i-1)%tabsize));
			}
		}
		return(i);
}

char* get_cur()
{
		return(line_start+x_offset);
}

#ifdef STATICBUF

void init_undo()
{
		undosp = undop = (void*)&unbuf;
		undop->link = NULL;
		undop->pos = cur_pos - 1;
		undop->length = 0;
}

static char ub[2] = "";

U_REC* new_undo(char *pos, int len)
{
	U_REC *tmp;
	int tlen = abs(undosp->length);

	/* wrap undo pointer if no space for requested len in undo buffer */
	tmp = (char*) &undosp->str[tlen] + sizeof(U_REC) + len < (char*) &unbuf + AMAX
			? (char*)&undosp->str[tlen]
			: (void*)&unbuf;
	tmp->link = undosp;
	tmp->pos = pos;
	tmp->length = 0;
	if(flag[CHG] && ((char*)tmp->link > 
		(char*)&unbuf + (AMAX - (AMAX / 10)))){
		bell();
		show_gets("Undo buffer 90\% full! ", (char*)ub, 1, show_pos);
	}
	return tmp;
}

void u_del(char *pos, int len)
{
		if(!undosp) init_undo();

		if(!undosp->length) undosp->pos = pos;
		/* get new_undo if top rec is u_ins, or different pos or no space */
		else if((undosp->length > 0) ||
		   ( (void*)&undosp->pos[0 - undosp->length] != pos ) ||		/* stack only del not bs */
		   ( (char*)&undosp->str[0 - undosp->length] + len >= (char*)unbuf + AMAX )) {
				undosp = new_undo(pos, abs(len));
				if(undosp == (void*)&unbuf) undo_wrap--;		/* say undo wrapped */
		}

		if(len) memcpy(&undosp->str[abs(undosp->length)], pos, len);
		undosp->length = undosp->length - len;

		if(!in_undo) { undop = undosp; undone = 0;}
}

void u_ins(char *pos, int len)
{
		int tlen;
		if(!undosp) init_undo();

		if(!undosp->length) undosp->pos = pos;
		/* get new_undo if top rec is u_del, or different pos or no space */
		else if((undosp->length < 0) ||
		   /* stack only left to right insertions */
		   ( (void*)&undosp->pos[undosp->length]  != pos ) ||
		   ( (char*)&undosp->str[undosp->length] + len >= (char*)unbuf + AMAX )) {
				undosp = new_undo(pos, abs(len));
				if(undosp == (void*)&unbuf) undo_wrap--;		/* say undo wrapped */
		}
		tlen = abs(undosp->length);

		if(len) memcpy(&undosp->str[tlen], pos, len);
		undosp->length = tlen + len;
		if(!in_undo) { undop = undosp; undone = 0;}
}

void ytot_ins()
{
		char* e;
		for(e=undop->pos; e<undop->pos + abs(undop->length); e++)
				if(*e == EOL) ytot++;
}

void undo()
{
		/* dont undo nothing, but say nothing is unchanged */
		if(undone) { bell(); return; }
		if(undop != undosp) undop = undop->link;
		in_undo++;
		goto_ptr(undop->pos);
		if(undop->length < 0) { /* is delete so insert */
				/* delete flag is negative length */
				file_resize(undop->pos, undop->pos - undop->length);
				memcpy(undop->pos, &undop->str[0], 0 - undop->length);
				u_ins(undop->pos, 0 - undop->length);
				ytot_ins(); 	/* adjust ytot when inserting */
		}
		else {			/* is insert so delete */
				file_resize(undop->pos + undop->length, undop->pos);
		}
		goto_ptr(undop->pos);
		if(!undop->link) {
				undone = -1;
				flag[CHG] = 0;
				show_top();
		}
		in_undo--;
		flag[SHW] = 1;
}

void redo()
{
		/* dont redo nothing */
		if(undop == undosp) { bell(); return; }
		in_undo++;
		goto_ptr(undop->pos);

		if(undop->length > 0) { /* is insert so insert */
				file_resize(undop->pos, undop->pos + undop->length);
				memcpy(undop->pos, &undop->str[0], undop->length);
				u_ins(undop->pos, undop->length);
				ytot_ins(); 	/* adjust ytot when inserting */
		}
		else {			/* is delete so delete */
				file_resize(undop->pos - undop->length, undop->pos);
		}
		goto_ptr(undop->pos);

		undop = (void*)&undop->str[abs(undosp->length)];
		undone = 0;
		in_undo--;
		flag[SHW] = 1;
}

#else /* STATICBUF */

char * realloc_buffer(int mode)
{
	char *ptr;
	int i;
	switch(mode) {

		case 0: {
			i = amax+AMAX+tabsize+1;
			if (i<0) return NULL;
			ptr = realloc(edbuf, (size_t)i);
			if (ptr) {
				amax += AMAX;
				i = ptr - edbuf;
				edbuf += i;
				file_end += i;
				cur_pos += i;
				line_start += i;
				last_pos += i;
				old_pos += i;
				screen_start += i;
				mk += i;
				bstart += i;
				bend += i;
				if (last_mk) last_mk += i;
			}
			return ptr;
		}

		case 1: {
			i = bmax+BMAX+tabsize+1;
			if (i<0) return NULL;
			ptr = realloc(bb, (size_t)i);
			if (ptr) {
				bmax += BMAX;
				bb = ptr;
			}
			return ptr;
		}

		case 2: {
			i = umax+UMAX+1;
			if (i<0) return NULL;
			ptr = realloc(unbuf, (size_t)i);
			if (ptr) {
				umax += UMAX;
				i = (int)ptr - (int)unbuf;
				unbuf = ptr;
				undop = (void*)undop+i;
				undosp = (void*)undosp+i;
			}
			return ptr;
		}
	}
	return NULL;
}

void init_undo()
{
		undosp = undop = unbuf;
		undop->link = VOID_LINK;
		undop->pos = cur_pos - edbuf -1;
		undop->length = 0;
}

U_REC* new_undo(char *pos, int len)
{
	U_REC *tmp;
	int tlen = abs(undosp->length);

 retry:
	/* wrap undo pointer if no space for requested len in undo buffer */
	if ((void *)undosp->str + tlen + sizeof(U_REC) + len < unbuf + umax) {
	      	tmp = (void *)undosp->str + tlen;
	} else {
		if (realloc_buffer(2)) goto retry;
		tmp = (void *)unbuf;
	}

	tmp->link = (unsigned int)undosp - (unsigned int)unbuf;
	tmp->pos = pos - edbuf;
	tmp->length = 0;
	return tmp;
}

void u_del(char *pos, int len)
{
		if(!undosp) init_undo();

		if(!undosp->length) undosp->pos = pos - edbuf;
		/* get new_undo if top rec is u_ins, or different pos or no space */
		if((undosp->length > 0) ||
		   ( undosp->pos + edbuf - undosp->length != pos) ||		/* stack only del not bs */
		   ( (void*)undosp->str - undosp->length + abs(len) >= unbuf + umax )) {
				undosp = new_undo(pos, abs(len));
				if(undosp == unbuf) undo_wrap--;		/* say undo wrapped */
		}

		if(len) memcpy(undosp->str + abs(undosp->length), pos, len);
		undosp->length = undosp->length - len;

		if(!in_undo) { undop = undosp; undone = 0;}
}

void u_ins(char *pos, int len)
{
		int tlen;
		if(!undosp) init_undo();

		if(!undosp->length) undosp->pos = pos - edbuf;
		/* get new_undo if top rec is u_del, or different pos or no space */
		if((undosp->length < 0) ||
		   /* stack only left to right insertions */
		   ( undosp->pos + edbuf + undosp->length != pos ) ||
		   ( (void *)undosp->str + undosp->length + abs(len) >= unbuf + umax )) {
				undosp = new_undo(pos, abs(len));
				if(undosp == unbuf) undo_wrap--;		/* say undo wrapped */
		}
		tlen = abs(undosp->length);
		if(len) memcpy(undosp->str + tlen, pos, len);
		undosp->length = tlen + len;
		if(!in_undo) { undop = undosp; undone = 0;}
}

void ytot_ins()
{
		char* e;
		for(e=undop->pos+edbuf; e<undop->pos+edbuf + abs(undop->length); e++)
				if(*e == EOL) ytot++;
}

void undo()
{
		/* dont undo nothing, but say nothing is unchanged */
		if(undone) { bell(); return; }
		if(undop != undosp) {
			if (undop->link == VOID_LINK)
				undop = unbuf;
			else
				undop = undop->link + unbuf;
		}
		in_undo++;
		goto_ptr(undop->pos + edbuf);
		if(undop->length < 0) { /* is delete so insert */
				/* delete flag is negative length */
				file_resize(undop->pos + edbuf, undop->pos + edbuf - undop->length);
				memcpy(undop->pos + edbuf, undop->str, - undop->length);
				u_ins(undop->pos + edbuf, - undop->length);
				ytot_ins(); 	/* adjust ytot when inserting */
		}
		else {			/* is insert so delete */
				file_resize(undop->pos + edbuf + undop->length, undop->pos + edbuf);
		}
		goto_ptr(undop->pos + edbuf);
		if(undop->link==VOID_LINK) {
				undone = -1;
				flag[CHG] = 0;
				show_top();
		}
		in_undo--;
		flag[SHW] = 1;
}

void redo()
{
		/* dont redo nothing */
		if(undop == undosp) { bell(); return; }
		in_undo++;
		goto_ptr(undop->pos + edbuf);

		if(undop->length > 0) { /* is insert so insert */
				file_resize(undop->pos + edbuf, undop->pos + edbuf + undop->length);
				memcpy(undop->pos + edbuf, undop->str, undop->length);
				u_ins(undop->pos + edbuf, undop->length);
				ytot_ins(); 	/* adjust ytot when inserting */
		}
		else {			/* is delete< so delete */
				file_resize(undop->pos + edbuf - undop->length, undop->pos + edbuf);
		}
		goto_ptr(undop->pos + edbuf);

		undop = (void*)undop->str + abs(undosp->length);
		undone = 0;
		in_undo--;
		flag[SHW] = 1;
}

#endif /* STATICBUF */

#ifdef TERMINAL
void sys_exit(int code)
{
	if(!(flag[CHG])) {
		/* watch where you're goin', clean up where you been */
		if(cfdpath) free(cfdpath);
		gotoxy(0,phys_height+2);
		ttclose();
		exitf = 0;
	} else putch(7);
}

void bell()
{
		putch(7);
}
#else
char a[2] = "";
void do_exit(int code)
{
	if((a[0] & ~0x20) == 'S') {do_save(); sys_exit(0);}
	if((a[0] & ~0x20) == 'X') {flag[CHG] = 0; sys_exit(0);}
}

void sys_exit(int code)
{
	// if file modified then do 'Save-as:', optionally save-and-exit.
	if(flag[CHG]) {
		bell();
		show_gets("Changed: Save, eXit or Cont.?", a, sizeof(a), &do_exit);
		return;
	}
	if(!(flag[CHG])) {
		/* watch where you're goin', clean up where you been */
		if(selection_text) free(selection_text);
		if(cfdpath) free(cfdpath);
#ifdef DOGTK
		gtk_exit(code);
#else
		exit(code);
#endif //DOGTK
	}
	bell();
}
#endif /* TERMINAL */

void goto_xpos()
{
	goto_x(x_offset < strlen(line_start+1)+1 ? 
			x_offset : 
			strlen(line_start+1)+1);
}

/* cursor movement */
void cursor_up()
{
	if(flag[BLK]) flag[SHW] = 1;
	if(ytru == 0) return;
	ytru--;
	while(*--line_start != EOL) ;
	y--;
	goto_xpos();
}

void cursor_down()
{
	if(flag[BLK]) flag[SHW] = 1;
	if(ytru == ytot) return;
	ytru++;
	while(*++line_start != EOL) ;
	y++;
	goto_xpos();
}

void move_to(int newx, int newy)
{
	int old;
	if ((newy < 0) || (file_end==edbuf+1)) return;
	if((y == newy && x == newx) || (executive != MAIN)) return;

	while((y < newy) && (ytru < ytot)) cursor_down();
	while((y > newy) && (ytru > 0)) cursor_up();

	if((line_start[x_offset] == EOL) && (x_offset != 1)) cursor_left();

	/* detect impossible to achieve x pos within tabs */
	old = xtru - newx;
	while((old > 0 ? xtru > newx : xtru < newx) && (*cur_pos != EOL)){
		xtru < newx ? cursor_right() : cursor_left();
	}
}

void getx()
{
	xtru = get_tru(cur_pos);
	x_offset = cur_pos - line_start;
	x = xtru - xlo;
}

/* cursor left & right */
void cursor_left()
{
	if(xtru == 1){ cursor_up(); goto_x(strlen(line_start+1)+1); return;};
	if(flag[BLK]) flag[SHW] = 1;
	if( x == 1 && xlo != 0){
		xlo -= XINC;
		flag[SHW] = 1;
	}
	--cur_pos;
	getx();
}

void cursor_right()
{
	if(*cur_pos == EOL) {goto_x(1); cursor_down(); return;};
	if(flag[BLK]) flag[SHW] = 1;
	if(x == screen_width-1) {
		xlo += XINC;
		flag[SHW] = 1;
	}
	++cur_pos;
	getx();
}

void cursor_pageup()
{
	int i,ytmp=y;

	for(i=1; i<screen_height; ++i) cursor_up();
	if(ytru < screen_height) ytmp = 0;
	y = ytmp;
	flag[SHW] = 1;
}

void cursor_pagedown()
{
	int i,ytmp=y;

	for(i=1; i<screen_height; ++i) cursor_down();
	y = ytmp;
	flag[SHW] = 1;
}

void word_left()
{
	char *d = cur_pos;

	if(d[-1] == EOL) { /* at bol */
		if(ytru == 0) return; /* don't backup past start */
		cursor_up();
		goto_x(strlen(line_start+1)+1); /* goto end of line */
		return;
	}

	/* skip white space separators */
	while(strchr(wdelims,d[-1])) { d--; cursor_left(); }
	/* skip real chars */
	while(!strchr(wdelims,d[-1])) { d--; cursor_left(); }
}

void word_right()
{
	char *d = cur_pos;

	if(*d == EOL) { /* at eol */
		if(ytru >= ytot) return;	/* don't go past end */
		cursor_down();
		goto_x(1);					/* goto start of line */
		return;
	}
	/* skip real chars */
	while(!strchr(wdelims,*d)) { d++; cursor_right(); }
	/* skip white space seperators */
	while(strchr(wdelims,*d)) { d++; cursor_right(); }
}

void word_mark()
{
	if(cur_pos[0] <= BLNK) return;	/* don't mark nothing */
	mark_off();
	if(!strchr(wdelims,cur_pos[-1])) word_left();
	block_mark();
	while(!strchr(wdelims,*cur_pos)) cursor_right();
#ifndef TERMINAL
	keve->state |= ShiftMask;
#endif /* TERMINAL */
}

// mark word under cursor and open in new window if a valid filename
void word_marknopen()
{
	char sbuf[NLEN];
	char *s = sbuf;
	struct stat o_stat;

	get_mk(sbuf);	// get just the filename
#ifndef TERMINAL
	s += strlen(sbuf);
	if(!stat(sbuf,&o_stat) && S_ISREG(o_stat.st_mode)){
		// pick up possible grep ':nnn:' string for implicit edx goto line
		while(!strchr(mdelims,*cur_pos)){ *s++ = *cur_pos; cursor_right();}
		*s = 0;
		new_edit(cfdpath, sbuf);
	}
#endif /* TERMINAL */
}

/* display --------------------------------------------------------*/
/* assuming cursor in correct position: show_rest(screen_width-x,line_start+x_offset) */

void drstr(char* disp, int i)
{
#ifdef TERMINAL
	disp[i] = 0;
	cputs(disp);
#else
	drawstring(disp,i);
#endif /* TERMINAL */
	outxy.X += i;
}

void show_rest(int len, char *s)
{
	char *ss;
	char disp[MAXVLINE];
	int i,j,k=0,xlt=xlo;

	ss = s;
	i = 0;
	if(flag[BLK]) {
		if(cur_pos < mk)
			{ bstart = cur_pos; bend = mk; }
		else
			{ bstart = mk; bend = cur_pos; }
		if ((ss >= bstart) && (ss < bend)) highvideo();
	}
	while(*ss != EOL && i < MAXVLINE) {
		if(flag[BLK]) {
			if (ss == bstart) { drstr(disp+xlo,i-xlo); k += i; xlo=i=0; highvideo(); }
			if (ss == bend) { drstr(disp+xlo,i-xlo); k += i; xlo=i=0; lowvideo(); }
		}
		if(*ss == '\t') {
			for(j = i +(tabsize - (k+i)%tabsize); i<j; disp[i++] = ' ');
			ss++;
		}
#ifdef TERMINAL
		else if(*ss == '\r') {disp[i++] = '.';ss++;}
#endif /* TERMINAL */
		else disp[i++] = *ss++;
	}
	if(i) drstr(disp+xlo,i-xlo);
	if(flag[BLK] && (ss == bend)) lowvideo();
	clreol();
	xlo = xlt;
}

/* line_start and y correct */
void show_scr(int fr, int to)
{
	int len=screen_width-1;
	int i=fr;

	screen_start=line_start;

	/* calculate screen_start */
	for(; i<y; i++) while(screen_start > edbuf && *--screen_start != EOL) ;
	for(; i>y; i--) while(*++screen_start != EOL) ;

	/* first line */
	screen_start++;
	do {
		gotoxy(0,fr+y2);
		if(screen_start<file_end && strlen(screen_start) > xlo)
			show_rest(len, screen_start);
		else {
			if(((flag[BLK]) &&
				(screen_start == bend)) ||
				(screen_start == bend+1)) lowvideo();
			clreol();
		}
		/* skip EOL */
		while(*screen_start++) ;
	} while(++fr <= to);
}

void scroll_up()
{
	if(!y) cursor_down();
	y--;
	flag[SHW] = 1;
}

void scroll_down()
{
	if(line_start <= edbuf) return;
	if(ytru <= y)  return;
	if(y >= screen_height-1) cursor_up();
	y++;
	flag[SHW] = 1;
}

void show_sdn(int line)
{
	gotoxy(0,y2+line);
	show_scr(0, screen_height-1);
}

void show_flag(int x, int g)
{
	gotoxy(20+x-1,y1);
	putch(g? fsym[x]: '.');
	flag[x] = g;
}

/*
  simple minded line input executive for status line dialogs.
  Successful completion, indicated by a Return key, executes the
  callback in dialogCB. Responds to 'Esc' and ^C by terminating
  without executing the callback. Printing first char resets Col.
  Use ^H to overwrite preset string, 'End' displays cursor to EOS.
  All other characters input to diabuf. Set by 'show_gets'.
*/

void dialog(int key)
{
	int skey;
	skey = key & 0xff;

	gotoxy(diastart+col,y1);
#ifdef TERMINAL
	if(key == 0x0c) {	/* ^L */
#else
	undraw_cursor();
	if(keve->state & ControlMask) skey &= 0x1f;
	if(key == XK_End) {
#endif /* TERMINAL */
		while((diabuf[col] != 0) && (col < dblen)) putch(diabuf[col++]);
	} else {
		switch(skey){
			case 8: {
				if(col < 0) col = strlen(diabuf);
#ifdef TERMINAL
				if(col) { col--; gotoxy(--outxy.X,outxy.Y); putch(' '); gotoxy(outxy.X,outxy.Y);}
#else
				if(col) { col--; outxy.X--; putch(' '); outxy.X--;}
#endif /* TERMINAL */
				break;
			}
			case  3:
			case 13:
			case 27: {
				executive = MAIN;
				diabuf[col] = 0;
				if(skey == 13) dialogCB(0);
				flag[SHW] = 1;
				scr_update();
				show_top();
				break;
			}
			default: {
				if(col < 0 || !first) {
					col = 0;
					gotoxy(diastart+col,y1);
					clreol();
				}
				if(col < dblen){
					diabuf[col++] = key;
					putch(key);
				} else bell();
			}
		}
	}
	first = key;
#ifndef TERMINAL
	draw_cursor();
	show_vbar();
#endif /* TERMINAL */
}

/* display a dialog property string at fixed location in the status bar */

void show_note(char *prp)
{
	gotoxy(29,y1);
	clreol();
#ifdef TERMINAL
	gotoxy(29,y1);
#endif /* TERMINAL */
	cputs(prp);
#ifdef TERMINAL
	diastart = outxy.X+strlen(prp)+2;
#else
	diastart = outxy.X+2;
	show_vbar();
#endif /* TERMINAL */
}

/* setup status line interactive dialog and input data processing callback */

int show_gets(char *prp, char *buf, int blen, void *cb)
{
	diabuf = buf;		// point at the input buffer
	dialogCB = cb;		// setup the dialog callback
	first = 0;			// first = 0 = True;
	dblen = blen;		// set max dialog buffer input char count

	lowvideo(); 		// make dialog stand out in status line
	show_note(prp); 	// show function
	cputs(": ");
	cputs(diabuf);		// show preset, if any
	col=strlen(diabuf); // point cursor at _end_ of preset string.
	executive = DIALOG; // tell handle_key() to pass keys to dialog
#ifndef TERMINAL
	draw_cursor();
#endif /* TERMINAL */
	return 0;
}

/* update line and col */
void show_pos()
{
	char tbuf[NLEN];

	if(executive != MAIN) return;
	highvideo();
	gotoxy(5,y1);
	snprintf(tbuf, sizeof tbuf, "%d %d     ", ytru+1, xtru);
	cputs(tbuf);
}

/* redraw the status line only in executive == MAIN mode */

void show_top()
{
	int i;
	char tbuf[NLEN];

#ifdef TERMINAL
	if(executive != MAIN || !exitf) return;
#else
	if(executive != MAIN) return;
#endif /* TERMINAL */
	gotoxy(0,y1);
	highvideo();
	clreol();
	show_pos();
	for(i=0; i<= SHW-1; i++)
		show_flag(i, flag[i]);
#ifdef TERMINAL
	snprintf(tbuf, sizeof tbuf, "   Help=^KH  %s", cfdpath);
#else
	snprintf(tbuf, sizeof tbuf, "   %s", cfdpath);
#endif /* TERMINAL */
	cputs(tbuf);
	lowvideo();
	gotoxy(x-1,y+y2);
}

/* display help/about page */

#ifdef TERMINAL
void show_help()
{
	int myy=1;
	char* prntstr = HELP_STR;
	if(!mk) mk = (void*) -1;
	clrscr();
	show_top();
	gotoxy(0,y2);
	{
		/* line at a time to process EOL char */
		while(prntstr < HELP_STR + sizeof(HELP_STR)){
			puts(prntstr);
			prntstr += strlen(prntstr)+1;
			gotoxy(0,++myy);
		};
	}
	help_done = -1;
	if(mk == (void*) -1) mk = 0;
}
#else
void show_help(int mode)
{
#ifdef EXTHELP
	char name[NLEN], *ptr;
	ptr=getenv("HOME");
	snprintf(name, sizeof name, "%s/%s", ptr, "MANUAL.ws");
	new_edit(cfdpath, name);
#else
	if(!mk) mk = (void*) -1;
	clrscr();
	show_top();
	gotoxy(0,y2);
	if (mode == 0) {
		int i; char* prntstr = HELP_STR;
		/* char at a time to process EOL char */
		for(i=0; i < sizeof(HELP_STR); putch(prntstr[i++]));
	}
	if (mode == 1) {
                int i, j;
		char *ptr = "Alt-key bindings";
		for (j=0; j<strlen(ptr); j++) putch(ptr[j]);
		putch(EOL); putch(EOL);
		for(i=0; i<26; i++) if ((ptr=binding[i])) {
			putch('a'+i); putch(':'); putch(' ');
			for (j=0; j<strlen(ptr); j++) putch(ptr[j]);
			putch(EOL);
		}
	}
	help_done = -1;
	if(mk == (void*) -1) mk = 0;
#endif /* EXTHELP */
}
#endif /* TERMINAL */

/*
  set editor options executive.
  Accept a single UPPER or lower case char only from the string "MFOCTBA":
	Modified, Fill, Overwrite, Case, Tab, Block marked, replace All.
*/

void options(int key)
{
	char *d;
	int k;

	k = toupper(key) & ~0x20;
	if((d=strchr(fsym, k)) != 0) {
		k = d-fsym;
		show_flag(k, !flag[k]);
	}
	executive = MAIN;
	show_top();
	scr_update();
}

/* setup 'executive = OPTIONS' mode */

void show_mode()
{
	highvideo();
	show_note(fsym);
	lowvideo();
	putch(BLNK);
	executive = OPTIONS;
}

/* file operation ---*/

void file_read()
{
#ifndef STATICBUF
	int d;
#endif /* STATICBUF */
	char *col = file_new();
	char c;
	int sz = 1;

#ifndef TERMINAL
	// dont do any file input on non-existant filename
	if(((fi == 0) && !first_time) ||
		((fi == 0) && first_time && *ewin.name)) goto badex;
	/* read complete line */
	do {
		if(first_time && !fi && !*ewin.name) sz = read((int)fi, (char*)&c, 1);
		else c = fgetc(fi);
		if(sz == -1) goto badex; // no stdin if sz == -1
#else
	if((fi == 0) || !*ewin.name) goto badex;
	/* read complete line */
	do {
		c = fgetc(fi);
#endif
		if(c == EOF || !sz) {
			if(fi) fclose(fi);
			fi = 0; /* no more read */
			break;
		}
		if(c == '\t' && flag[TAB]) { /* conditionally convert tabs on input */
			do (*file_end++ = BLNK);
			while( ((file_end-col) % tabsize) != 0);
		}
		else
			if(c == LF) {
			*file_end++ = EOL;
			col = file_end;
			ytot++;
		}
		else *file_end++ = c;
#ifdef STATICBUF
	} while(file_end < edbuf+AMAX-BMAX || c != LF);
#else
		if (file_end >= edbuf+amax) {
			d = file_end - col;
			realloc_buffer(0);
			col = file_end - d;
		}
	} while(file_end < edbuf+amax);
#endif /* STATICBUF */
	for(; ewin.jump>1; ewin.jump--) cursor_down();
badex:
	first_time = 0;
}

/* compress one line from end */

char *file_ltab(char *s)
{
	char *d, *e;

	e = d = strchr(s--, EOL);
	while(*--e == BLNK) ;	/* trailing blank */
	while(e > s) {
		if(e[0] == BLNK && (e-s)%tabsize == 0 && e[-1] == BLNK) {
			*--d = 9;
			while(*--e == BLNK && (e-s)%tabsize != 0) ;
		}
		else *--d = *e--;
	}
	return d;
}

/* routine used for write block file also, this makes it more complicated */

int file_write(int fd, char *s, char *e)
{
	if(fd == 0) return 1; /* no write */
	do {
		if(flag[TAB] && *s != EOL)
			s = file_ltab(s);
		if (*s && write(fd, s, strlen(s)) < 0)
			return 1;
		if (write(fd, "\n", 1) < 0)
			return 1;
		while(*s++ != EOL) ;
	} while(s < e);
	return 0;
}

int file_fout(void)
{
	if(fo == 0) {
		strlcpy(bbuf, ewin.name, sizeof bbuf);
		strlcat(bbuf, "XXXXXX", sizeof bbuf);
		if(!(fo=mkstemp(bbuf))) return -1;
	}

#ifndef MSVC
	if (!flag[NEW]) {
		fchmod(fo, ofstat.st_mode & 07777);
	}
#endif /* MSVC */
	return file_write(fo, edbuf+1, file_end);
}

void do_save()
{
	char bakfile[NLEN+1];

	/* prompt on empty filename */
	if(ewin.name[0] == 0) { file_save(-1,0); return;}

	executive=MAIN;
	if(file_fout() ) {
		bell();
		return; /* exit if can't write output */
	}

	fchmod(fo, 0644);
	close(fo);
	fo = 0; 					/* set fo to known condition */

	/* if editing old file then delete old file */
	if (flag[NEW] == 0) {
		strlcpy(bakfile, ewin.name, sizeof bakfile);
		strlcat(bakfile, "~", sizeof bakfile);
		unlink(bakfile);
		rename(ewin.name, bakfile);
#ifdef NOBAK
		unlink(bakfile);
#endif
	}

	rename(bbuf, ewin.name);	/* ren new temp to old name */
	flag[CHG] = 0;
	show_top();
}

/* go to top of file and reset to known condition */
void top()
{
	y1 = YTOP;
	y2 = y1+1;
	line_start = edbuf;
	x_offset = 1;
	xtru = x = 1;
	ytru = y = 0;
	flag[SHW] = 1;
}

#ifndef TERMINAL
void set_title(char *str)
{
	char b[NLEN];

	snprintf(b, sizeof b, "%s -"EDIT, str);
	XStoreName(dpy, win, b);
}
#endif /* TERMINAL */

/* new file init */
char *file_new()
{
	top();
	edbuf[0] = EOL;
	last_pos = cur_pos = file_end = edbuf+1;
	ytot = 0;
	flag[NEW] = 0;
	mark_off();
	last_mk = NULL;
	if((fi = fopen(ewin.name, "r")) == 0) flag[NEW]++;
	else fstat(fileno(fi), &ofstat);
	return(file_end);
}

#ifndef TERMINAL
/* exec the forking cmd */
int SYSTEM (char *cmd)
{
		#define SHELL	"/bin/sh"
		pid_t pid;
		switch(pid=fork()) {
			case 0:
				execl(SHELL,"sh","-c",cmd,(char *) 0);
				printf("Execl failed!\n");
				_exit(127);
			case -1: printf("fork error\n");
			default: return(0);
		}
}

void gorun()
{
	SYSTEM(Command);
}

/* chdir to specified directory and fire up another copy of edx */
void new_edit(char *path, char *fn)
{
	char b[1024];
	snprintf(b, sizeof b, "cd %s; "EDIT" %s", path ? path : "./", fn ? fn : newbuf );
	SYSTEM(b);
}

void newedit(char *fn)
{
	new_edit(cfdpath, fn);
}

#ifdef MINIMAL
/* chdir to working directory and fire up rxvt */

void mterm()
{
	char b[1024];
	snprintf(b, sizeof b, "cd %s;rxvt -font 8x16 -ls -sl 500 -sr +st -geometry 96x28 &",
			cfdpath ? cfdpath : "./");
	SYSTEM(b);
}
#endif /* MINIMAL */
#endif /* TERMINAL */

void do_cd()
{
	chdir(cfdpath);
}

void chgdir()
{
	show_gets("cd", cfdpath, BUFSIZ, do_cd);
}

void do_open()
{
	file_read();
#ifndef TERMINAL
	set_title(ewin.name);
#endif /* TERMINAL */
	executive=MAIN;
}

void file_save(int f_all, int f_win)
{
	if(((flag[CHG]) | f_all)) {
		show_gets("Save as", ewin.name, sizeof(ewin.name), &do_save);
		return;
	}
	/* optionally make new empty file */
	if(f_win) {
		show_gets("Open", ewin.name, sizeof(ewin.name), &do_open);
	}
}

#ifndef TERMINAL
void newfile()
{
	/* get new file name */
	show_gets("Open", newbuf, sizeof(newbuf), newedit);
}
#endif /* TERMINAL */

/* main editor workhorse. Inserts/deletes specified source/destination */
void file_resize(char *s, char *d)
{
#ifdef STATICBUF
	char	*e = file_end;
	unsigned i = e-s;
#else
	char	*e;
	unsigned int i;
	int s_rel = s - edbuf;
	int d_rel = d - edbuf;
#endif /* STATICBUF */

	/* immediate problem only when block buffer on disk too big */
#ifdef STATICBUF
	if((file_end += (d-s)) >= edbuf+AMAX) {
#else

	i = file_end - s;
	file_end += d-s;
retry:
	if(file_end>= edbuf+amax) {
		if (realloc_buffer(0)) { 
			s = edbuf + s_rel;
			d = edbuf + d_rel;
			goto retry; 
		}
#endif /* STATICBUF */
		show_note("Main buffer full!");
		bell();
		return;
	}
#ifndef STATICBUF
	e = s+i;
#endif /* STATICBUF */
	if(s < d) { /* expand */
		d += e - s;
		s = e;
		while(i-- > 0) *--d = *--s;
	}
	else {
		u_del(d, s - d);		/* save undo delete string */
		/* adjust ytot when shrink */
		for(e=d; e<s; e++) if(*e == EOL) ytot--;
		while(i-- > 0) *d++ = *s++;
	}
	*file_end = EOL;	/* last line may not complete */
	if(!flag[CHG] ) {
		show_flag(CHG, 1);
		gotoxy(x-1,y+y2);
		clreol();
	}
}

/* search and goto */

/* xx >= 1, yy >= 0 */

void goto_x(int xx)
{
	cur_pos = line_start + xx;
	if (cur_pos>file_end) cur_pos=file_end;
	xtru = get_tru(cur_pos);
	if(!xtru) cur_pos--;
	x_offset = cur_pos - line_start;

	xx = xlo;
	if(xtru == 1) xlo = 0;
	if(xtru-xlo >= screen_width)
		xlo = ((xtru + XINC - screen_width)/XINC) * XINC;
	x = xtru - xlo;
	if (xlo!= xx) flag[SHW]=1;
}

void goto_y(int yy)
{
	int i, n;

	n = ytru;

	for(i=n; i<yy; i++) cursor_down();
	for(i=yy; i<n; i++) cursor_up();
	if(y > screen_height || y < 0) {
		flag[SHW] = 1;
		y = screen_height/4;
	}
}

void goto_ptr(char *s)
{
	/* find line_start <= s */
	char *s1 = s;
	int yo = y;

#ifdef STATICBUF
	if(s < edbuf || s >= edbuf + AMAX) {
#else
	if(s < edbuf || s >= edbuf + amax) {
#endif /* STATICBUF */
		bell();
		return;
	}
	top();
	while(*--s1 != EOL && s1 > edbuf) ; 				/* find start of target line */
	while(line_start+1 <= s1 && y < ytot) cursor_down();/* move to target line */
	while(line_start+1 > s && y > 0) cursor_up();		/* move back before target */
	goto_x(s-line_start);								/* get x from line start */
	if(y > screen_height || xlo != 0) {
		flag[SHW] = 1;
		y = yo;
	}
}

void goline()
{
	goto_y(atoi(lbuf)-1);
}

void goto_last()
{
	mark_off();
	goto_ptr(last_pos); cursor_down();
}

void goto_line()
{
	show_gets("Goto line", lbuf, sizeof(lbuf), goline);
}

#ifndef TERMINAL
void run()
{
	show_gets("Command", Command, sizeof(Command), gorun);
}
#endif /* TERMINAL */

void gocol()
{
	goto_x(atoi(cbuf) );
}

void goto_col()
{
	show_gets("Goto Column", cbuf, sizeof(cbuf), gocol);
}

void twist_pos()
{
	char *c;
	if (!last_mk || last_mk>=file_end) return;
	c = cur_pos;
	goto_ptr(last_mk);
	last_mk = c;
	if (mk) mk = last_mk;
}

/* compare to sbuf. used by search */
int str_cmp(char *s)
{
	char	*d = sbuf;

	if(flag[CAS] )
	{
		while(*d ) if(*s++ != *d++ ) return 1;
	}
	else
	{
		while(*d ) if(tolower(*s++) != tolower(*d++)) return 1;
	}
	return 0;
}

/* back/forward search */
char *goto_find(char *s, int back)
{
	int slen = strlen(sbuf);
	mark_off();
l1:
	do
	{
		if(back) {
			if(--s <= edbuf) {
				bell();
				return 0;
			}
		}
		else {
			if(++s >= file_end) {
				bell();
				return 0;
			}
		}
	} while(str_cmp(s) );

	if( flag[WRD] && ((!strchr(wdelims,s[-1])) || (!strchr(wdelims,s[slen]))) )
		goto l1;

#ifdef TERMINAL
	goto_ptr(s+slen);
	block_mark();
	last_mk = mk -= slen;
#else
	goto_ptr(s);
	block_mark();
	last_mk = mk += slen;
#endif /* TERMINAL */
	return s;
}

void gofind()
{
	goto_find(cur_pos, 0);
}

void get_mk(char* dbuf)
{
	if(strchr(sdelims,*cur_pos) && !flag[BLK]) return;
	memset(dbuf,0,NLEN * sizeof(char));
	if(!flag[BLK]) word_mark();
	if(flag[BLK] && (mk != cur_pos) && (labs(cur_pos - mk) < NLEN)){
		memcpy(dbuf, mk < cur_pos ? mk : cur_pos, labs(cur_pos - mk)); 
	}
}

char *goto_search(int back)
{
	get_mk(sbuf);
	show_gets("Search for", sbuf, sizeof(sbuf), gofind);
	return NULL;
}

void dorepl()
{
	rlen = strlen(rbuf);
	do {
		file_resize(s+slen, s); 		/* delete srch string&save undo data */
		file_resize(s, s+rlen); 		/* insert space for replace string */
		memcpy(s, rbuf, rlen);
		u_ins(s, rlen); 				/* add replace string to undo buf */
		s = s+rlen;						/* skip over new string */
	}
	while(flag[ALL] && (s=goto_find(s, 0)) != 0) ;
	flag[SHW] = 1;
}

void doSAR()
{
	if((s =(char*) goto_find(cur_pos,0))) {
		slen = strlen(sbuf);
		show_scr(0, screen_height-1);
		gotoxy(x-1,y+y2);
		highvideo();
		{
			int i = slen;
			char *sb=s;
			while(i-- && !putch(*sb++));
		}
		lowvideo();
		show_gets("Replace with", rbuf, sizeof(rbuf), dorepl);
	}
}

void goto_replace(int all)
{
	get_mk(sbuf);
	show_gets("SAR for", sbuf, sizeof(sbuf), doSAR);
}

void gettab()
{
		tabsize = atoi(twbuf);
}

void tab_size()
{
	show_gets("Tab size",twbuf, sizeof(twbuf), gettab);
}

void getmarg()
{
	screen_width = atoi(wbuf);
	flag[SHW]++;
}

void window_size()
{
	show_gets("Wrap col",wbuf, sizeof(wbuf), getmarg);
}

/* block and format ---*/
/* use blen, mk, bb */

void mark_off()
{
	mk = NULL;
	flag[BLK] = 0;
	flag[SHW] = 1;
	show_top();
}

void reset_mark()
{
	if (!last_mk) return;
	flag[BLK] = 1; mk = last_mk; 
}

void block_mark()
{
	if( mk == NULL ) {
		mk = cur_pos;
		last_mk = mk;
		flag[BLK] = 1;
		show_top();
	}
	else
		mark_off();
}

void block_put()
{
#ifdef STATICBUF
	if(blen < BMAX) memcpy(bb, mk, blen);
#else
	while (blen >= bmax && realloc_buffer(1));

	if (blen < bmax)
		memcpy(bb, mk, blen);
#endif /* STATICBUF */
	else {
		if(fb == 0 && (fb = tmpfile()) == 0) return;
		fseek(fb, 0L, 0);
		fwrite(mk, 1, blen, fb);
	}
}

void block_get()
{
	int i;

#ifdef STATICBUF
	if(blen < BMAX) memcpy(mk, bb, blen);
#else
	while (mk+blen >= edbuf+amax && realloc_buffer(0));
	if (mk+blen < edbuf+amax) 
		memcpy(mk, bb, blen);
#endif /* STATICBUF */
	else {
		if(fb == 0) return;
		fseek(fb, 0L, 0);		/* 0L offset, 0=beginning of file */
		fread(mk, 1, blen, fb);
	}
	/* calculate ytot */
	for(i=0; i<blen; i++) {
		if((mk[i] == EOL) || (mk[i] == LF)) {
			ytot++;
			mk[i] = 0;
		}
	}
}

void block_line()
{
	if(ytru == ytot) return;
	goto_x(1);
	for(blen = 0; line_start[++blen] != EOL; ) ;
	mk = line_start+1;
	block_put();

	/* delete line and save to undo buf */
	file_resize(line_start+blen, line_start);
	show_sdn(y);
	mark_off();
}

void remove_block()
{
	char *s;

	if(!flag[BLK]) return; 		/* exit if no mark */
	if(cur_pos < mk) {		/* ensure cur_pos >= mk */
		s = cur_pos ;		/* swap cur_pos & mk */
		cur_pos = mk ;
		mk = s ;
	}
	s = cur_pos;
	goto_ptr(mk);

	/* delete block and save to undo buf */
	file_resize(s, mk);
	cur_pos = mk;
}

void block_remove_update()
{
	remove_block();
	mark_off();
}

void chg_case(int upper)
{
	char *s;
	int i;

	if(!flag[BLK]) return; 	/* exit if no mark */
	if(cur_pos < mk) {		/* ensure cur_pos >= mk */
		s = cur_pos ;		/* swap cur_pos & mk */
		cur_pos = mk ;
		mk = s ;
		goto_ptr(cur_pos);
	}
	i = cur_pos+1 - mk;		/* get length */
	u_del(mk, i-1);			/* save undo data enmasse */
	while(i--) mk[i] = !upper ? tolower(mk[i]) : toupper(mk[i]);
	u_ins(mk, cur_pos - mk);
	flag[SHW] = flag[CHG] = 1;
}

void transpose()
{
	char c;
	if (cur_pos && cur_pos>edbuf+1 && cur_pos<file_end) {
		u_del(cur_pos-1, 2);
		c = *cur_pos;
		*cur_pos = *(cur_pos-1);
		*(cur_pos-1) = c;
		u_ins(cur_pos-1, 2);
		flag[SHW] = flag[CHG] = 1;
	}
}

void block_copy(int delete)
{
	char *s;

	if(!flag[BLK]) return; 		/* exit if no mark */
	if(cur_pos < mk) {		/* ensure cur_pos >= mk */
		s = cur_pos ;		/* swap cur_pos & mk */
		cur_pos = mk ;
		mk = s ;
	}
	blen = cur_pos - mk;	/* get length */
	block_put();			/* copy block to buffer */
	if(delete) {
		block_remove_update();
		return;
	}
	mark_off();
}

void block_paste()
{
	if(!blen) return;			/* dont paste nothing */
	if(flag[BLK]) block_remove_update();		/* delete marked block */

	file_resize(cur_pos, cur_pos+blen);
	mk = cur_pos;
	block_get();

	/* save to undo buf */
	u_ins(mk - 1, blen);
	goto_ptr(mk + blen);
	mark_off();
}

/* read file into cursor position */
void doblockr()
{
	char *bb_end;
#ifdef STATICBUF
	int c;
#else
	int c, d;
#endif /* STATICBUF */
	char *col;

	if(!(fb = fopen(fbuf, "r"))) { bell(); return; };

	fseek(fb, 0L, 2);		/* seek to 0L offset, 2=end of file */
	blen=ftell(fb); 		/* get file length */
	fseek(fb, 0L, 0);		/* seek back to file start */
	bb_end = col = bb;

	/* read complete file */
	do {
		c = fgetc(fb);
		if(c == EOF) {
			fclose(fb);
			fb = 0; /* no more read */
			break;
		}
		if(c == '\t' && flag[TAB]) {
			do (*bb_end++ = BLNK);
			while( ((bb_end-col) % tabsize) != 0);
		}
		else
			if(c == LF) {
				*bb_end++ = EOL;
				col = bb_end;
			}
		else
			*bb_end++ = c;
#ifdef STATICBUF
	} while(bb_end < bb+BMAX || c != EOF);
#else
		if (bb_end >= bb+bmax) {
			d = (int)bb;
			realloc_buffer(1);
			d = (int)bb - d;
			col += d;
			bb_end += d;
		}
	} while(bb_end < bb+bmax);
#endif /* STATICBUF */

	blen = bb_end - bb;
	block_paste();
}

void block_read()
{
	show_gets("Read file", fbuf, sizeof(fbuf), doblockr);
}

/* copy block to file, not to block buffer */
void doblockw()
{
	int	 fd;

	show_flag(TAB, 0);
	fd = open(fbuf, O_RDWR|O_CREAT, 0644);
	file_write(fd, bstart, bend);
	close(fd);
	show_top();
}

void block_write()
{
	if(!flag[BLK]) return;
	if(cur_pos < mk)
		{ bstart = cur_pos; bend = mk; }
	else
		{ bstart = mk; bend = cur_pos; }

	show_gets("Write file", fbuf, sizeof(fbuf),(void*) doblockw);
}

/* fill current line */
int block_fill()
{
	goto_x(screen_width);
	while((cur_pos > line_start+1) && ((*cur_pos == EOL)||(*cur_pos > BLNK))) cursor_left();
	if(cur_pos == line_start+1)
		goto_x(screen_width-1);
	else
		key_delete();
	key_return();
	goto_x(strlen(line_start+1)+1);
	flag[CHG] = 1;
	return cur_pos - line_start;
}

/* format paragraph */
void block_format()
{
	char	*s=line_start;

	goto_x(1);
	/* remove newlines to end of para */
	while(s = strchr(++s, EOL), ytru < ytot && s[1] != EOL) {
		u_del(s,1);
		*s = BLNK;
		u_ins(s,1);
		ytot--;
		flag[CHG] = 1;
	}
	/* truncate line <= screen_width */
	while(goto_x(strlen(line_start+1)+1), x_offset >= screen_width) block_fill();
	/* goto end of line */
	while(line_start[x_offset] ) cursor_right();
	show_top();
	flag[SHW] = 1;
}

/* find and set cursor to matching bracket '(,{,[,],},)' */

void find_match()
{
	char *s = cur_pos;
	int dire, cnt;
	char ch, mch;

	if (!s) return;

	ch = *s;
	switch (ch) {
		case '(': mch = ')'; dire = 0; break;
		case ')': mch = '('; dire = 1; break;
		case '[': mch = ']'; dire = 0; break;
		case ']': mch = '['; dire = 1; break;
		case '{': mch = '}'; dire = 0; break;
		case '}': mch = '{'; dire = 1; break;
		default: return;
	}
	cnt = 0;
	if (dire) {
		while (--s >= edbuf) {
			if (*s == ch) cnt++;
			else if (*s == mch) {
				if (!cnt) goto match;
				cnt--;
			}
		}
	} else {
		while (++s < file_end) {
			if (*s == ch) cnt++;
			else if (*s == mch) {
				if (!cnt) goto match;
				cnt--;
			}
		}
	}
	return;
match:
	goto_ptr(s);
	flag[SHW]=1;
}

/* key actions ---*/

void key_deleol(char *s)
{
	if(ytru == ytot) return;
	if(flag[BLK]) {
		block_remove_update();		/* delete marked block */
		return;
	}
	goto_ptr(s);

	/* delete eol and save to undo buf */
	file_resize(s+1, s);
	flag[SHW]=1;
}

/* delete char under cursor */
void key_delete()
{
	char	*s=cur_pos;

	if(flag[BLK]) {
		block_remove_update();
		return;
	}
	else if( *s == EOL) {
		key_deleol(s);
		return;
	}
	/* delete cursor char and save to undo buf */
	file_resize(s+1, s);
	show_rest(screen_width-x, s);
}

/* delete char before cursor */
void key_backspace()
{
	char *s=cur_pos;

	if(flag[BLK]) {
		block_remove_update();
		return;
	}
	if(*--s == EOL) { /* delete EOL */
		if(ytru == 0) return;
		cursor_up();
		goto_x(strlen(line_start+1)+1);
		key_deleol(s);
		return;
	}
	cursor_left();
	key_delete();
	flag[SHW]=1;
}

/* delete non-white string + white-space string */
void key_delword(int eol)
{
	char	*d=cur_pos;

	if(flag[BLK]) {
		block_remove_update();
		return;
	}
	if(*d == EOL) {
		key_deleol(d);
		return;
	}
	if(eol) while(*d != EOL) d++;
	else {
		while(!strchr(sdelims,*d) && *d != EOL) d++;
		while(strchr(s1delims,*d) && *d != EOL) d++;
	}
	/* delete word and save to undo buf */
	file_resize(d, cur_pos);
	flag[SHW]=1;
}

/* insert tab char */
void key_tab(int tabi)
{
	if(flag[BLK]) {
		key_delete();
	}

	if(flag[TAB]){
		do key_normal(' ');
		while((xtru%tabsize) != 1 && *cur_pos != EOL);
	} else {
		key_normal('\t');
	}
}

/* insert newline, increase line count */
void key_return()
{
	char	*s = cur_pos;

	/* reset marked block on char entry */
	if(flag[BLK]) {
		key_delete();
		s = cur_pos;
	}
	else if(flag[OVR] ) {
		cursor_down();
		goto_x(1);
		return;
	}
	file_resize(s, s+1);
	goto_x(1);
	*s = EOL;
	/* save newly inserted EOL to undo buf */
	u_ins(s, 1);
	ytot++;
	/* get prev line_start */
	s = line_start+1;
	cursor_down();
	goto_x(1);	/* ensure visible cursor */
	if(flag[IND]) {
		int i=0;
		/* count spaces and tabs */
		while(s[i] == BLNK || s[i] == '\t') i++;
		if(i){ /* and only if */
			file_resize(line_start+1, line_start+1+i);
			memcpy(line_start+1,s, sizeof(char) * i);
			/* save undo info */
			u_ins(line_start+1, i);
			x_offset += i;
			goto_x(i+1);
		}
	}
	flag[SHW]=1;
}

/* everything ASCII else */
void key_normal(int key)
{
	char *s=line_start+x_offset;
	int xtmp;

	/* reset marked block on char entry */
	if(flag[BLK]) { key_delete(); s = cur_pos; }

	if(flag[OVR] && *s != EOL) {
		/* save old overwrite char to undo buf */
		u_del(s, 1);
		putch(*s = key);
		flag[CHG] = 1;
	}
	else {
		file_resize(s, s+1);
		*s = key;
		flag[SHW] =1 ;
	}
	cursor_right();
	/* save new overwrite/insert char to undo buf */
	u_ins(s, 1);

	if(!flag[FIL] || xtru < screen_width) return;

	xtmp = block_fill();	/* cursor_down */
}

/* simple, aint it? */
void rec_macro(int k)
{

	if(k & 0xff00) bell();	/* beep&discard non ASCII, ie func keys */
	else					/* else record key */
#ifdef TERMINAL
		*pmbuf++ = k;
#else
		*pmbuf++ = keve->state & ControlMask ? k & 0x1f: k;
#endif /* TERMINAL */
}

/* turn on macro recording or play back macro */
void key_macros(int record)
{
	char *s=mbuf;
	int k;

	if(!record) {	/* play macro back */
		if(*s == 0) { bell(); return; };	/* ding user on empty mbuf */
		doCtrlX = 0;		/* turn off control key state from ^K */
#ifndef TERMINAL
		keve->state = 0;	/* turn off control key state from ^P */
#endif /* TERMINAL */
		while(*s != 0) {
			k = 255 & *s++;
			switch(executive) {
				case MAIN:
					main_exec(k);
					break;
				case DIALOG:
					dialog(k);
					break;
				case OPTIONS:
					options(k);
					break;
			}
		}
		flag[SHW] = 1;
		return;
	}
	/* else toggle recording on/off */
	flag[REC] ^= 1;
	show_top();
	if(!flag[REC]) {	/* terminal condition */
		pmbuf[-2] = 0;	/* backup over ^K^M & terminate macro with a 0x0 */
		pmbuf = mbuf;	/* reset record pointer to start of mbuf */
	}
}

#ifdef GREEK
void key_greek(int key)
{
unsigned char c;
char *seq = NULL;
	int i, l;
	c = key | 0x40;
	if (c>='a' && c<='z')
		seq = greek_letter[c-'a'];
	if (c>='A' && c<='Z') 
	  	seq = greek_letter[c-'A'];
	if (seq) {
	  	key_normal('\\');
		if (*seq=='!') {
			++seq;
			if (var_greek && c>='a') {
				key_normal('v');			
				key_normal('a');			
				key_normal('r');
			}
  		}
		l = strlen(seq);
		if (c>='a')
			for (i=0; i<l; i++) key_normal(tolower(seq[i]));
		else
			for (i=0; i<l; i++) key_normal(seq[i]);
	}
	var_greek = (c == 'v' || c=='V');
}
#endif
