
#ifdef DOGTK
#define VERSION "Gtk 1.05 (OpenBSD)"
#else //DOGTK
#define VERSION "1.05 (OpenBSD)"
#endif //DOGTK

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <pwd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#define  XK_MISCELLANY
#include <X11/keysymdef.h>
#include <sys/time.h>
#ifdef DOGTK
#include <gtk/gtk.h>
#include <gtk/gtksocket.h>
#include <gdk/gdkx.h>
#endif //DOGTK

#define DEFAULT_WIDTH 80
#define DEFAULT_HEIGHT 25

//#define GENERIC
#ifndef GENERIC

//the following defines are for personal settings
// EXTHELP for external help file ~/MANUAL.ws
//#define EXTHELP
// VERTCURS for a vertical cursor
#define VERTCURS
// TWOBUTN paste with button3 click, set selection with button3 drag
#define TWOBUTN
// MINIMAL remove /usr/share/edx and edxrc
#define MINIMAL
// NOBAK remove bak files.
#define NOBAK
// STATICBUF forces edbuf, bb and unbuf to static fixed location/sizes.
//#define STATICBUF
// THREED gives the scrollbar 'thumb' depth
#define THREED

// DARK is for the hacker in you
//#define DARK
#ifdef DARK

// yellow char on black, white cursor, black on grey high-lights
char *FgColor="Yellow", *BgColor="Black", *CrColor="White", *HiFgColor="Black", *HiBgColor="Grey75";

// bold font for yellon on black with no cursor artifacts
#define FONTNAME	"-b&h-lucidatypewriter-medium-r-normal-sans-12-*-*-*-m-70-iso8859-1"

#else

// black char on linen, red cursor, black on grey high-lights
char *FgColor="Black", *BgColor="Linen", *CrColor="Red", *HiFgColor="Black", *HiBgColor="Grey75";

// medium font for black on linen best presentation
#define FONTNAME	"-b&h-lucidatypewriter-medium-r-normal-sans-12-*-*-*-m-70-iso8859-1"
#endif /* DARK */

/* turns on display of keypressed event values in upper right corner */
//#define DEBUG

#else

/* screen color defines */
//char *FgColor="Black", *BgColor="Linen", *CrColor="Red", *HiFgColor="Black", *HiBgColor="Grey75";
char *FgColor="Yellow", *BgColor="Black", *CrColor="White", *HiFgColor="Black", *HiBgColor="Grey75";

//#define FONTNAME	"-b&h-lucidatypewriter-medium-r-normal-sans-12-*-*-*-m-70-iso8859-1"
#define FONTNAME	"-b&h-lucidatypewriter-bold-r-normal-sans-12-*-*-*-m-70-iso8859-1"
//#define FONTNAME "8x13"
//#define FONTNAME "9x15bold"

#endif /* GENERIC */

#define AMAX	0xD0000 	/* main buffer size */
#define BMAX	0x10000 	/* block size */
#define YTOP	0			/* first line */

#define EOL '\0'	/* end of line marker */
#define BLNK	' ' /* blank */
#define LF	'\n'	/* new line */
#define NLEN	256 /* input buffer length */
#define XINC	20	/* increment for x offset */

#define CHG 0	/* file Modified: =0 file not changed, !=0 file changed */
#define FIL 1	/* Fill */
#define OVR 2	/* character insert=0, Overwrite=1 */
#define CAS 3	/* Case sensitive: =0 no, !=0 yes */
#define TAB 4	/* Tab expand */
#define BLK 5	/* block mark active */

#define IND 6	/* auto indent flag */
#define REC 7	/* recording macro flag */
#define ALL 8	/* replace all flag */
#define WRD 9	/* find wdelims delimited word */
#define SHW 10  /* update & show entire screen */
#define NEW 11	/* editing a new file */
#define WIN 12	/* window: <0 same win, =0 load new */

/************************************************************/

/* use MS style COORD to track cursor pos */
typedef struct {
	int X;	/* x coordinate */
	int Y;	/* y coordinate */
} COORD;	/* x-y structure */

COORD outxy;	/* cursor coordinates for screen positioning */

/************************************************************/

/* character handler types */
static enum {
	MAIN,
	DIALOG,
	OPTIONS
} executive = MAIN; 	/* default character handler */

/* X related globals */
#ifdef DOGTK
GtkWidget *main_win;
guint32 xid = 0;
#endif //DOGTK
Display *dpy;
Window win;
GC gc;
XFontStruct *font;
XEvent event;
XKeyEvent *keve;
Time eve_time;
char *selection_text;	/* selected text for X clipboard */
int selection_length;
int do_background = -1;

int Width, Height;
int fwidth, fheight;	/* font character width, height */

/* Foreground, Background normal and highlight colores */
XColor FgXColor, BgXColor, CrXColor, HiFgXColor, HiBgXColor;

int HiLo;			/* hightlight status, 0=off */
Atom DeleteWindow;		/* Atom of delete window message */

char *FontName=NULL;
char *DisplayName=NULL;
char *AppName;
char *RcFile = NULL;

char Command[NLEN] = "fsx  ";
char *cfdpath;

char *Geometry=NULL;
/* maximum viewable 6x8 chars on a 2000x1600 screen */
#define MAXVLINE 2048
char eolbuf[MAXVLINE];	/* holds spaces for clreol(), 450 is 2000x1600 */

/* func prototypes */

void gotoxy(int horz,int vert);
void cursor_draw(unsigned long color);
void draw_cursor();
void undraw_cursor();
void drawstring(char *str, int len);
void cputs(char *prntstr);
int putch(char chr);
void clreol();
void highvideo();
void lowvideo();
void clrscr();
void bell();
void update();
void show_vbar();
void scroll_text();
void child_sig_handler(int signal);
void sig_handler(int);
void font_height(void);
int paste_primary(int win, int property, int Delete);
int request_selection(int time);
char *mrealloc(char *s, int len);
void set_selection();
void send_selection(XSelectionRequestEvent * rq);
void init(int argc,char *argv[]);
void handle_key(char *astr, int skey, int state);
void moveto();
void do_paste();
void do_select(int delete);
int main(int argc,char *argv[]);

/* for monolithic whole include edit engine here */

#define EDIT "edx"
#ifndef MINIMAL
#define SHARE_DIR "/etc"
#define DEFAULT_RC "edxrc"
#endif /* MINIMAL */

#ifdef EMACS
#include "emacs.c"
#endif
#ifdef WORDSTAR
#include "ws.c"
#endif

/******************** Start of cursor I/O ********************/

/* Goto the specified location. */

void gotoxy(int horz,int vert)
{
	outxy.X = horz;
	outxy.Y = vert;
}

void cursor_draw(unsigned long color)
{
	XSetForeground(dpy, gc, color);
#ifndef VERTCURS
	XFillRectangle(dpy, win, gc,
		(outxy.X * fwidth) + 2, ((outxy.Y+1)* fheight) + 2, fwidth-1, 2);
#else
	XDrawRectangle(dpy, win, gc,
		(outxy.X * fwidth) + 2, (outxy.Y * fheight) + 2, 0, fheight);
#endif /* VERTCURS */
	XSetForeground(dpy, gc, HiLo ? HiFgXColor.pixel : FgXColor.pixel);
}

void draw_cursor()
{
	cursor_draw( CrXColor.pixel);
}

void undraw_cursor()
{
	cursor_draw( HiLo ? HiBgXColor.pixel : BgXColor.pixel);
}

void drawstring(char *str, int len)
{
	if (outxy.X+len>screen_width+1) len = screen_width+1-outxy.X;
	XDrawImageString(dpy, win, gc, (outxy.X * fwidth) + 2, (outxy.Y+1) * fheight, str, len > 0 ? len : 0);
}

int putch(char chr)
{
	static char str[]="\0\0";

	if(!(chr) || ((chr & 0xff) == LF)){
		clreol();
		outxy.X = 0;
		outxy.Y += 1;
	} else {
		str[0] = chr;
		drawstring(str, 1);
		outxy.X += 1;
	}

	return 0;
}

void cputs(char *prntstr)
{
	int strl = strlen(prntstr);

	drawstring(prntstr, strl);
	outxy.X += strl;
}

/* Erase from cursor to end of line. */

void clreol()
{
	int tmpx = outxy.X;
	int eollen = abs(screen_width+1 - tmpx);
	outxy.X = outxy.X <= screen_width ? outxy.X : screen_width;
#ifdef THREED
	drawstring(eolbuf, eollen+1);
#else
	drawstring(eolbuf, eollen);
#endif /* THREED */
	outxy.X = tmpx;
}

/* display string if visible, bump X */

void highvideo()
{
	XSetBackground(dpy,gc,HiBgXColor.pixel);
	XSetForeground(dpy,gc,HiFgXColor.pixel);
	HiLo = -1;
}

void lowvideo()
{
	XSetBackground(dpy,gc,BgXColor.pixel);
	XSetForeground(dpy,gc,FgXColor.pixel);
	HiLo = 0;
}

void clrscr()
{
	XClearWindow(dpy,win);
}

void bell()
{
	XBell(dpy,100);
}

void update()
{
	flag[SHW] = 1;
	show_top();
	scr_update();
}

int vtot, vcur, pos, ftheight;

void show_vbar()
{
	int j;

	// display buffer screen height in pixels
	vtot = (screen_height-2)*fheight;

	// font true height as a constant
	ftheight = fheight+font->descent+1;

	// thumb height is display buffer percentage of total lines
	vcur = (vtot*(screen_height-1))/((ytot)?ytot:1);

	// min thumb size is 1 char
	if (vcur<fheight) vcur = fheight;

	// max thumb size is 2/3 vtot
	j = 2*vtot/3;
	if (vcur>j) vcur = j;

	// current line number to vertical thumb pos
	pos = (ytru*(vtot-(vcur-fheight)))/((ytot)?ytot:1);

	// draw scrollbar trough
	XSetForeground(dpy,gc,HiBgXColor.pixel);
#ifdef THREED	// provide background for line scroll triangles
	XFillRectangle(dpy, win, gc, Width-11, 0, 11, Height);
#else	// indicate line scroll rectangles
	XFillRectangle(dpy, win, gc, Width-11, ftheight, 11, Height+2-2*(ftheight));
#endif /* THREED */

	// draw thumb
#ifdef DARK
		XSetForeground(dpy,gc,~HiFgXColor.pixel);
#else
		XSetForeground(dpy,gc,FgXColor.pixel);
#endif /* DARK */
	XDrawRectangle(dpy, win, gc, Width-10, ftheight+pos, 9, vcur);

#ifdef THREED
	{
		XPoint opoints[] ={{Width-11,Height},{0,-Height},
						   {-(Width-11),0},{0,ftheight-1},
						   {Width,0},	// draw boxes
						   {-5,-10},{-5,10},{0,Height+2-(2*(ftheight))},
						   {5,10},{5,-10},{-11,0}	// draw triangles
						  };
		XSegment lpoints[] ={{Width-1, ftheight+pos, Width-10, ftheight+pos},
							 {Width-10, ftheight+pos, Width-10, ftheight+pos+vcur-1},
							 {Width-10, ftheight-2, Width-5, ftheight-11},
							 {Width-9, Height+2-ftheight, Width-6, Height+3-ftheight+6}
							};

		// outline trough and end triangles
		XDrawLines(dpy, win, gc, opoints, 11,CoordModePrevious);

		// draw in light color for 3D shading
#ifdef DARK
		XSetForeground(dpy,gc,BgXColor.pixel);
#else
		XSetForeground(dpy,gc,BgXColor.pixel);
#endif /* DARK */
		XDrawSegments(dpy, win, gc, lpoints, 4);
	}
#endif /* THREED */
	lowvideo();
}

void scroll_text(int ycur)
{
	int m, newy;

	m = ycur - ftheight;
	newy = (m*ytot)/vtot;

	if(m<0) scroll_down();					// if cursor at top of screen
	else if(m>vtot+ftheight) scroll_up();	// if cursor at bottom of screen
	else if(m<pos) cursor_pageup();			// if cursor above thumb
	else if(m>pos+vcur) cursor_pagedown();	// if cursor below thumb
	else goto_y(newy);						// else cursor on thumb so track it

	scr_update(); // does show_vbar
}

void child_sig_handler(int signal)
{
    if (signal == SIGCHLD) wait(NULL);
}

void sig_handler(int nothing)
{
	update();
	XFlush(dpy);
}

int paste_primary(int win, int property, int Delete)
{
	Atom actual_type;
	int actual_format, i;
	long nitem, bytes_after, nread;
	unsigned char *data;
	char indent = flag[IND];	/* stash autoindent state */

	if (property == None)	/* don't paste nothing */
	return(0);

	flag[IND] = 0;	/* off autoindent */
	nread = 0;
	/* X-selection paste loop */
	do {
		if (XGetWindowProperty		/* get remaining selection max 1024 chars */
			(dpy, win, property, nread / 4, 1024, Delete,
			 AnyPropertyType, &actual_type, &actual_format, &nitem,
			 &bytes_after, (unsigned char **) &data)
			!= Success)
			return(0);
		update_scr = 0; 		/* dont update scr...yet */
		/* paste last batch one char at a time */
		for(i = 0; i < nitem; handle_key(NULL, data[i++],0));
		nread += nitem;
		XFree(data);
	} while (bytes_after > 0);
	update_scr = 1; 		/* _now_ update display */
	flag[SHW] = 1;
	scr_update();
	flag[IND] = indent; /* restore autoindent state */
	return(nread);
}

int request_selection(int time)
{
	Window w;
	Atom property;

	if ((w = XGetSelectionOwner(dpy, XA_PRIMARY)) == None) {
		int tmp = paste_primary(DefaultRootWindow(dpy), XA_CUT_BUFFER0, False);
		return(tmp);
	}
	property = XInternAtom(dpy, "VT_SELECTION", False);
	XConvertSelection(dpy, XA_PRIMARY, XA_STRING, property, win, time);
	return(0);
}

char *mrealloc(char *s, int len)
{
	char *ttt;
	if(!s) ttt = (char *) malloc(len);
	else ttt = (char *) realloc(s, len);
	return ttt;
}

void set_selection()
{
	int i;

	if(!flag[BLK] || mk == cur_pos) return;

	if(cur_pos < mk)
		{ bstart = cur_pos; bend = mk; }
	else
		{ bstart = mk; bend = cur_pos; }

	selection_length = bend - bstart;
	if ((selection_text = (char *) mrealloc(selection_text, selection_length)) == NULL) {
		printf("realloc.\n");
se_exit:
		bell();
		return;
	}
	for (i = 0; i < selection_length; i++) {
		selection_text[i] = bstart[i] == EOL ? '\n' : bstart[i];
	}
	XSetSelectionOwner(dpy, XA_PRIMARY, win, (Time) eve_time);
	if (XGetSelectionOwner(dpy, XA_PRIMARY) != win) {
		printf("Cant select.\n");
		goto se_exit;
	}
	XChangeProperty(dpy, DefaultRootWindow(dpy), XA_CUT_BUFFER0,
		XA_STRING, 8, PropModeReplace, selection_text,
		selection_length);
}

void send_selection(XSelectionRequestEvent * rq)
{
	XSelectionEvent notify;

	notify.type = SelectionNotify;
	notify.display = rq->display;
	notify.requestor = rq->requestor;
	notify.selection = rq->selection;
	notify.target = rq->target;
	notify.time = rq->time;
	XChangeProperty(dpy, rq->requestor, rq->property, XA_STRING, 8,
		PropModeReplace, selection_text, selection_length);
	notify.property = rq->property;
	XSendEvent(dpy, rq->requestor, False, 0, (XEvent *) & notify);
}

#ifndef MINIMAL
void parse_rc(FILE *f)
{
	char buf[512], *ptr;
	int i, l;
	while ((ptr=fgets(buf, 510, f))) {
		if (*ptr == '#' || isspace(*ptr)) continue;
		i = (int)*(ptr++) - (int)'a';
		if (*ptr==':' && i>=0 && i<26) {
			++ptr;
			while(isspace(*ptr)) ++ptr;
			l = strlen(ptr)-1;
			if (l>=0 && ptr[l]=='\n') ptr[l] = '\0';
			if (binding[i]) free(binding[i]);
			binding[i] = strdup(ptr);
		}
	}
}

void read_rc()
{
	FILE *fr = NULL;
	char name[NLEN], *ptr;

	if ((ptr=getenv("HOME")) || RcFile) {
		if (RcFile) {
			if (ptr && *RcFile == '~' && RcFile[1] == '/')
			snprintf(name, sizeof name, "%s/%s", ptr, RcFile+2);
			else
			strlcpy(name, RcFile, sizeof name);
		}
		else
			snprintf(name, sizeof name, "%s/.%s", ptr, DEFAULT_RC);
	} else {
		snprintf(name, sizeof name, "%s/%s", SHARE_DIR, DEFAULT_RC);
	}
	fr = fopen(name, "r");
	if (fr) {
		parse_rc(fr);
		fclose(fr);
	}
}
#endif /* MINIMAL */

#ifdef DOGTK
static gboolean quit_gtk(GtkWidget *widget, GtkWidget *bw)
{
    gtk_main_quit();
    return 0;
}
#endif //DOGTK

void init(int argc,char *argv[])
{
	int screen;
	XWMHints *wmh;
	XSizeHints *xsh;
	XClassHint *classh;
	XColor tmp;
	int i, x = 1, y = 0;

	/* Default settings */
	memset((void *)binding, 0, 26*sizeof(char *));
	*ewin.name = '\0';
#ifndef STATICBUF
	amax = AMAX;
	bmax = BMAX;
	umax = UMAX;
#endif /* STATICBUF */
 
	/* engine screen width, height, font */
	screen_height = DEFAULT_HEIGHT - 1;
	screen_width = DEFAULT_WIDTH;

	/* get command line options */
	for (i=1; i<argc; i++) {
		if (i<argc-1 && !strcasecmp(argv[i], "-fn"))
			FontName = argv[++i];
		else if (i<argc-1 && !strcasecmp(argv[i], "-rc"))
			RcFile = argv[++i];
		else if (i<argc-1 && !strcasecmp(argv[i], "-t"))
			tabsize = atoi(argv[++i]);
		else if (i<argc-1 && !strcasecmp(argv[i], "-fg"))
			FgColor = argv[++i];
		else if (i<argc-1 && !strcasecmp(argv[i], "-bg"))
			BgColor = argv[++i];
		else if (i<argc-1 && !strcasecmp(argv[i], "-b"))
			do_background ^= -1;
		else if (i<argc-1 && !strcasecmp(argv[i], "-hifg"))
			HiFgColor = argv[++i];
		else if (i<argc-1 && !strcasecmp(argv[i], "-hibg"))
			HiBgColor = argv[++i];
		else if (i<argc-1 && !strcasecmp(argv[i], "-cur"))
			CrColor = argv[++i];
		else if (i<argc-1 && !strcasecmp(argv[i], "-j"))
			ewin.jump = atoi(argv[++i]);
#ifdef DOGTK
		else if (i<argc-1 && !strcasecmp(argv[i], "-x"))
			xid = atoi(argv[++i]);
#endif //DOGTK
		else if (i<argc-1 && !strcasecmp(argv[i], "-w")) {
			screen_width = atoi(argv[++i]);
			if (screen_width<20) screen_width = 20;
		} else if (i<argc-1 && !strcasecmp(argv[i], "-h")) {
			screen_height = atoi(argv[++i]);
			if (screen_height<5) screen_height = 5;
		} else {
			if (*ewin.name) break;
			{
				char* p = strchr(argv[i], ':');
				if(p) {
					*p++ = '\0';
					ewin.jump = atoi(p);
				}
			}
			strlcpy(ewin.name, argv[i], sizeof ewin.name);
		}
	}

#ifndef STATICBUF
	/* alloc memory for the main buffer and block buffer */
	edbuf = (char *) malloc((size_t)(amax+tabsize+1));
	bb = (char *) malloc((size_t)(bmax+tabsize+1));
	unbuf = (void *) malloc((size_t)(umax+tabsize+1));

	if (!edbuf || !bb || !unbuf) {
		fprintf(stderr,"Memory allocation failed, aborting !!\n");
		exit(1);
	}
#endif /* STATICBUF */

#ifdef DOGTK
   if (!xid) {
       main_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/* open the display */
	dpy=XOpenDisplay(DisplayName);
   }   
   else {
    main_win = gtk_plug_new(xid);
    gtk_window_set_title (GTK_WINDOW (main_win), "gdx");
    gtk_container_border_width (GTK_CONTAINER (main_win), 5);
    gtk_signal_connect(GTK_OBJECT(main_win), "destroy",
                      GTK_SIGNAL_FUNC(quit_gtk), NULL);
    gtk_widget_realize(main_win);
    win = GDK_WINDOW_XWINDOW(main_win->window);
    dpy = GDK_WINDOW_XDISPLAY(main_win->window);
   }
#else //DOGTK
	/* open the display */
	dpy=XOpenDisplay(DisplayName);
#endif //DOGTK

	if(dpy==NULL)  {
		fprintf(stderr,"Can't open display: %s\n",DisplayName);
		sys_exit(1);
	}

	screen=DefaultScreen(dpy);

	/* establish window manager hints data structure */
	xsh=XAllocSizeHints();
	wmh=XAllocWMHints();
	classh=XAllocClassHint();

	x = 1; y=0;

	/* setup font(s) */
	if(FontName == NULL) { FontName = FONTNAME; }

	font=XLoadQueryFont(dpy, FontName);
	if(font==NULL) {
		fprintf(stderr, "Fonts not found.\n");
		sys_exit(1);
	}

	fheight = font->ascent + font->descent;
	Height = fheight * screen_height + font->descent+3;

	/* font width */
	fwidth = XTextWidth(font,"8",1);

	Width = (fwidth * screen_width)+13;
	xsh->flags = PSize;
	xsh->width = Width;
	xsh->height = Height;

	/* initialize clreol string to all blanks */
	memset(eolbuf, ' ', sizeof(eolbuf));

	/* create the only window */
#ifdef DOGTK
	if(!xid)
	  win=XCreateSimpleWindow(dpy,RootWindow(dpy,screen),x,y,Width,Height,0,
		BlackPixel(dpy,screen),WhitePixel(dpy,screen));
    else {
    gtk_widget_set_usize (GTK_WIDGET (main_win),Width,Height);
    gtk_widget_show(main_win);
    }
#else //DOGTK
	win=XCreateSimpleWindow(dpy,RootWindow(dpy,screen),x,y,Width,Height,0,
		BlackPixel(dpy,screen),WhitePixel(dpy,screen));
#endif //DOGTK

	/* setup window hints */
	wmh->initial_state=NormalState;
	wmh->input=True;
	wmh->window_group = win;
	wmh->flags = StateHint | InputHint | WindowGroupHint;

	/* setup window class resource names */
	classh->res_name = (AppName==NULL)?"edx":AppName;
	classh->res_class = "Xedit";

	/* name that window */
	XmbSetWMProperties(dpy, win, "edx", "edx", argv, argc,
					xsh, wmh, classh);

	/* setup to gracefully respond to exit requests from X */
	DeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &DeleteWindow, 1);

	/* specify accepted XEvent loop events */
	XSelectInput(dpy, win,
			KeyPressMask|\
			FocusChangeMask|\
			StructureNotifyMask|\
			ButtonPressMask|\
			ButtonReleaseMask|\
			ExposureMask|\
			PropertyChangeMask|\
			Button1MotionMask|\
			Button2MotionMask|\
			Button3MotionMask|\
			VisibilityChangeMask
	);
	keve = (XKeyEvent *)&event;

	/* create the Graphic Context for drawing purposes */
	gc=XCreateGC(dpy,win,0,NULL);

	/* allocate required colors */
	XAllocNamedColor(dpy,DefaultColormap(dpy,screen),FgColor,&FgXColor,&tmp);
	XAllocNamedColor(dpy,DefaultColormap(dpy,screen),BgColor,&BgXColor,&tmp);
	XAllocNamedColor(dpy,DefaultColormap(dpy,screen),CrColor,&CrXColor,&tmp);
	XAllocNamedColor(dpy,DefaultColormap(dpy,screen),HiFgColor,&HiFgXColor,&tmp);
	XAllocNamedColor(dpy,DefaultColormap(dpy,screen),HiBgColor,&HiBgXColor,&tmp);

	/* apply colors to window */
	XSetForeground(dpy,gc,FgXColor.pixel);
	XSetWindowBackground(dpy,win,BgXColor.pixel);

	/* set the font */
	XSetFont(dpy,gc,font->fid);

	/* map the window real */
	XMapWindow(dpy, win);
}

void handle_key(char *astr, int skey, int state)
{
	char chstr[NLEN];
	int x=outxy.X, y=outxy.Y, n=0;

	/* display keyboard shift/control/alt status */
	highvideo();
	gotoxy(0,y1);
	for(n=5;n;chstr[n--]=' ');
	if(state & ShiftMask) chstr[n++] = 'S';
	if(state & Mod1Mask) chstr[n++] = 'A';
	if(skey & 0xff00) chstr[n++] = 'F';
	if(state & ControlMask) chstr[n++] = '^';
	chstr[n] = skey & 0xff;
	chstr[5] = '\0';
	cputs(chstr);
	chstr[0] = '\0';

#ifdef DEBUG
	/* display raw key event data */
	snprintf(chstr, sizeof chstr, "k=%4x,s=%2x.",skey,state);
#endif /* DEBUG */
	gotoxy(screen_width - 12,0);
	clreol();
	cputs(chstr);
	gotoxy(x,y);
	show_vbar();

	if((skey >= 0xffe1) && (skey <= 0xffee)) return;
	if(skey == NoSymbol) return;

	switch(executive) {
	case MAIN:
		main_exec(skey);
		break;
	case DIALOG:
		dialog(skey);
		break;
	case OPTIONS:
		options(skey);
		break;
	}
}

void moveto()
{
	move_to(((event.xbutton.x < 0 ? 0 : event.xbutton.x)/fwidth) + 1 + xlo,
		((event.xbutton.y-3)/fheight) - 1);
}

void do_select(int delete)
{
	if(flag[BLK] && mk != cur_pos) {
		set_selection();
		block_copy(delete);
	}
	mark_off();
}

void do_paste()
{
	if(flag[BLK] && executive == MAIN) block_remove_update();
	request_selection(event.xbutton.time);
}

int main(int argc,char *argv[])
{
	Atom WM_PROTOCOLS = 0;
	struct sigaction sig;
	struct sigaction act;
	int y0 = 0, y1 = 0, yf, yt;

#ifdef DOGTK
    gtk_init (&argc, &argv);
#endif //DOGTK

	init(argc,argv);
#ifndef MINIMAL
	read_rc();
#endif /* MINIMAL */

	/* disconnect from the console */
#ifdef DOGTK
	if(!xid && do_background)
#else
	if(do_background)
#endif //DOGTK
		{switch (fork()) { case 0: case -1: break; default: exit(0); }}

	/* set path */
	if (cfdpath == NULL) {
		cfdpath = (char *) malloc(BUFSIZ);
		getcwd(cfdpath, BUFSIZ);
		if (strcmp(cfdpath, "/") != 0)
			strlcat(cfdpath, "/", BUFSIZ);
	}

	usleep(10000);
	XFlush(dpy);
	do_open();

/* end of edit engine init */

	/* request/create WM_PROTOCOLS atom */
	WM_PROTOCOLS = XInternAtom(dpy, "WM_PROTOCOLS", False);

	// kill off zombie child processes
	act.sa_handler = child_sig_handler;
	act.sa_flags = 0;
	sigaction(SIGCHLD, &act, NULL);

	/* set up the signal handler response */
	sig.sa_handler=sig_handler;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags=SA_RESTART;
	sigaction(SIGALRM,&sig,NULL);

	clrscr();

#define DBLCLICK 500

	/* main event loop, dispatch function calls in response to events */
	for(;;)  {
		static Time button_release_time;
		static int buttonpressed = 0;

		XNextEvent(dpy,&event);

		switch(event.type)	{
		case Expose:
			while(XCheckTypedEvent(dpy,Expose,&event));
			update();
			break;
		case MotionNotify:
			if (buttonpressed==-1) {
		        if (executive == MAIN)
					scroll_text(event.xbutton.y);
				break;
			}
			if(buttonpressed) {
				if(!flag[BLK]) block_mark();
				undraw_cursor();
				if (event.xbutton.y < fheight+2) {
					moveto();
					cur_pos = get_cur();
					scroll_down();
					scr_update();
				} else if (event.xbutton.y >= Height-6-font->descent) {
					moveto();
					cur_pos = get_cur();
					scroll_up();
					scr_update();
				} else {
					moveto();
					yf = y0;
					if (y<yf) yf = y;
					if (y1<yf) yf = y1;
					yt = y0;
					if (y>yt) yt = y;
					if (y1>yt) yt = y1;
					show_scr(yf, yt);
					moveto();
					cur_pos = get_cur();
					gotoxy(x-1,y+1);
					draw_cursor();
					y1 = y;
				}
			}
			break;
		case ButtonPress:
			if((event.xbutton.button == Button5) || 
				(event.xbutton.button == Button4))
				break; /* ignore mouse wheel */
	        if (event.xbutton.x>=Width-10) {
				buttonpressed = -1;
		        if (executive == MAIN)
					scroll_text(event.xbutton.y);
				break;
			}
			buttonpressed = event.xbutton.button;
			if (event.xbutton.time - eve_time < DBLCLICK) break;
			eve_time = event.xbutton.time;
			if(event.xbutton.button == Button1) mark_off(); /* unmark */
			if(!mk){
				moveto();
				y0 = y;
				cur_pos = get_cur();
				block_mark();	/* start new mark */
				flag[SHW] = 1;
				scr_update();
			}
			break;
		case ButtonRelease:
			if (buttonpressed==-1) {
				buttonpressed = 0;
				break;
			}
			buttonpressed = 0;
			switch (event.xbutton.button) {
#ifdef TWOBUTN
				case Button5:
					if (executive == MAIN) scroll_up();
					goto scrupdt;
				case Button4:
					if (executive == MAIN) scroll_down();
					goto scrupdt;
				case Button1:
					if (event.xbutton.time - button_release_time < DBLCLICK) {
						mark_off(); word_marknopen(); scr_update();
						break;
					}
					else
					{
						button_release_time = event.xbutton.time;
						if(mk == cur_pos) mark_off();
						goto setcursor;
					}
					break;
				case Button3:
					if ((event.xbutton.time - eve_time < DBLCLICK) &&
						(event.xbutton.time - eve_time)) {
						do_paste();
						break;
					}
					else
					{
						button_release_time = event.xbutton.time;
						set_selection();
						mark_off(); 	/* unmark */
					}
					goto setcursor;
#else
				case Button1:
				case Button3:
					if (event.xbutton.time - button_release_time < DBLCLICK) {
						mark_off(); cursor_right(); word_left(); block_mark();	/* set mark to left end of word */
						word_right(); scr_update(); /* set mark to right end of word */
						break;
					}
					else
					{
						button_release_time = event.xbutton.time;
						set_selection();
						goto setcursor;
					}
					break;
#endif /* TWOBUTN */
				case Button2:
					if ((event.xbutton.time - eve_time < DBLCLICK) &&
						(event.xbutton.time - eve_time)) {
						do_paste();
					}
					goto setcursor;
			}
			break;
setcursor:
			moveto();
scrupdt:
			flag[SHW] = 1;
			cur_pos = get_cur();
			scr_update();
			break;
		case KeyPress:{
				int count;
				char astr[10];
				KeySym skey;
				{ int i = 128; while(XCheckTypedEvent(dpy,KeyPress,&event) && i--);}
				eve_time = keve->time;
				astr[0] = 0;
				count = XLookupString(keve, astr,
						  sizeof (astr), &skey, NULL);
				astr[count] = 0;
				handle_key(astr, skey, keve->state);
			}
			break;
		case SelectionClear:
			break;
		case SelectionRequest:
			send_selection((XSelectionRequestEvent *) &
				event);
			break;
		case SelectionNotify:
			paste_primary(event.xselection.requestor,
				  event.xselection.property, True);
			break;
		case ConfigureNotify:
			Width = event.xconfigure.width;
			screen_width = (Width-16)/fwidth;
			Height = event.xconfigure.height;
			screen_height = (Height/fheight) -1 ;
			update();
			break;
		case ClientMessage:
			if(event.xclient.message_type == WM_PROTOCOLS)	{
				if(event.xclient.data.l[0] == DeleteWindow) {
					sys_exit(0);
				};
			};
			break;
		case DestroyNotify:
			XCloseDisplay(dpy);
			exit(0);
		default:
			break;
		};
	};
}
