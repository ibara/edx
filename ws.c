
/*
	edx ( small ws clone EDitor for X), (C) 2003, Terry Loveall
	released into the public domain. This program comes with no warranties or
	binaries. Compile and use at your own risk.
*/

#ifndef EXTHELP
/* Help screen -- rewrite to suit your own tastes */

static char HELP_STR[] = "ws clone EDX (EDitor for X) Ver. "VERSION"\0\
Usage: edx [-fn font] [-j line#] [-t tab#] [-w width] [-h height] [-bg color]\0\
    [-fg color] [-hibg color] [-hifg color] [-cur color] [-rc rcfile] [file]\0\
^O[mfoctbnaw] (Mod, Fill, Ovrwrit, Case, Tab, Blk mk, Ndent, repl All, Word)\0\
*NOTE: M flag must be reset to exit or load new file.\0\
 F6 Chgdir   Alt-? show Alt key bindings    ^QX switch block marks\0\
^QS goto bol      ^KQ or ^KX exit    ^KH F1 show help  ^F1 Mrk&Opn Str\0\
^QD goto eol      ^KB mark block on  ^KR read file\0\
^QR start of file ^KK mark block off ^KF save as\0\
^QC end of file   ^KY cut block      ^KS F2 save & resume\0\
^QF find          ^KC copy block     ^KZ F10 rxvt\0\
^QA find & repl   ^KV paste block     F4 find matching\0\
^QY del to eol    ^KW write block     F5 exec command\0\
^QM get margin    ^KT get tab size   ^QP goto prev pos\0\
^QU uppercase blk ^QL lowercase blk  ^QT transpose characters\0\
Shift-Del cut     ^Ins copy      Shift-Ins paste   F3 open new file\0\
^A word left      ^R page up     ^Z scroll up      ^I insert tab\0\
^F word right     ^C page down   ^W scroll dn      ^J goto line\0\
^T del word       ^D right       ^V toggle insert  ^L repeat find\0\
^G del char       ^S left        ^M ^N put newline ^B reformat para\0\
^H del prev char  ^E up          ^KP play macro    ^U undo\0\
^Y delete line    ^X down        ^KM togl rec mac  Alt-U redo\0\
                   Press any key to continue ...";
#endif /* EXTHELP */

/* include the editor engine */
#include "eeng.c"

/* Wordstar bindings */

#ifdef GREEK
/* similar to Ctrl-C under emacs... */
void ctrlc_key(int key)
{
	key_greek(key);
	doCtrlC = var_greek;
}
#endif /* GREEK */

/* ctrl K keys */
void ctrlk_key(int key)
{
	switch(key | 0x60) {
		case 'b': block_mark(); break;		/* ^K^B set mark block on */
		case 'c': set_selection();			/* ^K^C block copy to X clipboard */
				  block_copy(0); break; 	/*	and to bbuffer */
		case 'd': flag[CHG]=0; break;		/* ^K^D say file not changed */
		case 'f': file_save(-1,0); break;	/* ^K^F save as */
#ifdef GREEK
		case 'g': doCtrlC = 1; break;		/* ^K^G Greek char */
#endif /* GREEK */
		case 'h': show_help(0); break;		/* ^K^H show help menu */
		case 'k': block_mark(); break;		/* ^K^K set mark block on */
		case 'm': key_macros(1); break; 	/* ^K^M record macro */
		case 'p': key_macros(0); break; 	/* ^K^P play macro */
		case 'q': sys_exit(0); break;		/* ^K^Q exit only if file saved */
		case 'r': block_read(); break;		/* ^K^R read file into cursor pos */
		case 's': do_save(); break; 		/* ^K^S save file and resume */
		case 't': tab_size(); break;		/* ^K^T get tab size */
		case 'v': block_paste(); break; 	/* ^K^V copy buffer to cursor */
		case 'w': block_write(); break; 	/* ^K^W write block to disk */
		case 'x': sys_exit(0); break;		/* ^K^X exit only if file saved */
		case 'y': block_copy(1); break; 	/* ^K^Y cut block to buffer */
#ifdef MINIMAL
		case 'z': mterm(); break;			/* ^K^Z open an rxvt term */
#endif
	}
	doCtrlK = 0;
	show_top();
}

/* ctrl Q keys */
void ctrlq_key(int key)
{
	switch(key | 0x60) {
		case 'a': goto_replace(0); break;	/* ^Q^A replace */
		case 'c': goto_y(ytot); 			/* ^Q^C eof */
		case 'd': goto_x(strlen(line_start+1)+1); break;	/* ^Q^D eol */
		case 'f': goto_search(0); break;	/* ^Q^F find */
		case 'i': goto_line(); break;		/* ^Q^I goto line */
		case 'l': chg_case(0); break; 		/* ^Q^L lower case block */
		case 'm': window_size(); break; 	/* ^Q^M get right margin */
		case 'p': goto_last(); break;		/* ^Q^P goto last pos */
		case 'r': top();					/* ^Q^R bof */
		case 's': goto_x(1); break; 		/* ^Q^S bol */
		case 't': transpose(); break;		/* ^Q^T transpose chars */
		case 'u': chg_case(1); break; 		/* ^Q^L upper case block */
		case 'x': twist_pos(); break;		/* ^Q^X switch between block marks */
		case 'y': key_delword(1); break;	/* ^Q^Y delete to end of line */
	}
	doCtrlQ = 0;
	show_top();
}

/* Alt key bindings */
void key_alt(int key)
{
#ifdef MINIMAL
	switch(key) {
		case 'c': SYSTEM("xcalc"); break;				/* Alt-C xcalc*/
		case 'd': newedit("/c/text/phone.dir"); break;	/* Alt-D phone Dir */
		case 'l': SYSTEM("xcalendar"); break;			/* Alt-L xcalendar */
		case 'u': redo(); break;						/* Alt-U redo */
	}
#else
	char b[1024];
	if (key=='?') { show_help(1); return; }
	if (key=='u') { redo(); return; }
	if (key>=0x20) key = (key|0x60)-'a';
	if (key>=0 && key<26) if (binding[key]) {
		snprintf(b, sizeof b, binding[key], (cfdpath? cfdpath : "./"), ewin.name);
		SYSTEM(b);
	}
#endif /* MINIMAL */
}

void oblk()
{
	 // if shift not pressed turn off marked block
	if(!(keve->state & ShiftMask) && flag[BLK]) mark_off();
}

/* Shift, control and bare Function keys */
void key_func(int key)
{
	if(keve->state & ShiftMask)
		switch(key) {
		case XK_Insert	: do_paste(); return; 			/* Shift Ins paste */
		case XK_Delete	: do_select(1); return;			/* Shift Del cut */
		}
	if((keve->state & ShiftMask) && !flag[BLK] &&
		key >= XK_Home && key <= XK_End) block_mark();	/* enable marked block with Shift */
	if(keve->state & ControlMask) {
		switch(key) {
		case XK_F1		: word_marknopen(); break;		/* ^F1 mark cursor word & open */
		case XK_F3		: file_save(0,1); break;		/* ^F3 open new file */
		case XK_Home	: top(); break; 				/* ^Home bof */
		case XK_End 	: goto_y(ytot); break;			/* ^End eof */
		case XK_Left	: word_left(); break;			/* ^Left word left */
		case XK_Right	: word_right(); break;			/* ^Right word right */
		case XK_Insert	: do_select(0); break;			/* ^Ins copy */
		}
	} else
		switch(key) {
		case XK_F1		: show_help(0); break;			/* F1 show help */
		case XK_F2		: do_save(); break; 			/* F2 save file and resume */
		case XK_F3		: newfile(); break;				/* F3 open new edx */
		case XK_F4		: find_match(); break;			/* F4 find matching bracket */
		case XK_F5		: run(); break; 				/* F5 get and run cmd */
		case XK_F6		: chgdir(); break; 				/* F6 get & change to dir */
		case XK_F7		: block_mark(); break;			/* F7 set mark block on */
		case XK_F8		: block_mark(); break;			/* F8 set mark block on */
#ifdef MINIMAL
		case XK_F10 	: mterm(); break;				/* F10 open rxvt in cur.dir */
#else
		case XK_F10 	: key_alt('z'); break;			/* F10 open rxvt in cur.dir */
#endif
		case XK_F12 	: show_flag(OVR, !flag[OVR]); break;/* Ins toggle insert mode */
		case XK_Return	: key_return(); break;			/* Enter newline at cursor */
		case XK_Tab 	: key_tab(0); break;			/* Tab insert tab char at cursor */
		case XK_BackSpace: key_backspace(); break;		/* BS delete prev char */
		case XK_Insert	: show_flag(OVR, !flag[OVR]); break;/* Ins toggle insert mode */
		case XK_Delete	: key_delete(); break;			/* Del delete cursor char */
		case XK_Page_Up : cursor_pageup(); break;		/* PgUp */
		case XK_Page_Down: cursor_pagedown(); break;	/* PgDn */
		case XK_End 	: goto_x(strlen(line_start+1)+1); break;/* End eol */
		case XK_Home	: goto_x(1); break; 			/* Home bol */
		case XK_Up		: cursor_up(); break;			/* up */
		case XK_Down	: cursor_down(); break; 		/* down */
		case XK_Right	: cursor_right(); break;		/* right */
		case XK_Left	: cursor_left(); break; 		/* left */
	}
	oblk(); // conditionally off block mark
}

/* Control keys */
void key_control(int key)
{
	int xkey=1;
	switch(key|0x60) {
		case 'b': block_format(); break;		/* reformat block */
		case 'g': key_delete(); break;			/* delete cursor char */
		case 'h': key_backspace(); break;		/* destructive backspace */
		case 'i': key_tab(0); break;			/* insert tab char */
		case 'j': keve->state & ControlMask ?
					goto_line() :				/* goto line# */
					key_return(); break;		/* newline */
		case 'k': doCtrlK = key; break; 		/* ^K key */
		case 'l': goto_find(cur_pos, 0); break; /* find again */
		case 'm': 
		case 'n': key_return(); break;			/* newline at cursor */
		case 'o': show_mode(); break;			/* change modes */
		case 'p': literal = 1; break;			/* get inline literal */
		case 'q': doCtrlQ = key; break; 		/* ^Q key */
		case 't': key_delword(0); break;		/* delete word/to word */
		case 'u': undo(); break;				/* undo */
		case 'v': show_flag(OVR,!flag[OVR]);
		case 'w': scroll_down(); break; 		/* scroll down */
		case 'y': block_line(); break;			/* delete line */
		case 'z': scroll_up(); break;			/* scroll up */
				  break;						/* toggle Insert mode */
		default:  xkey--;	// say nothing done
	}
	if(!xkey){
		oblk();	// off marked block unless shift key pressed
		switch(key|0x60) {
			case 'a': word_left(); break;			/* word left */
			case 'c': cursor_pagedown(); break; 	/* pgdn */
			case 'd': cursor_right(); break;		/* right */
			case 'e': cursor_up(); break;			/* up */
			case 'f': word_right(); break;			/* word right */
			case 'r': cursor_pageup(); break;		/* pgup */
			case 's': cursor_left(); break; 		/* left */
			case 'x': cursor_down(); break; 		/* down */
		}
	}
	show_top();
}

/* undraw, redraw status and display */
void scr_update()
{
	if(executive == MAIN){

#ifndef TERMINAL
		undraw_cursor();
#endif /* TERMINAL */
		/* update text buffer display */
		if( flag[BLK] ) flag[SHW] = 1;
		if(y <= -1 || y >= screen_height) {
			if(y == -1) {
				y++; show_sdn(0);
			}
			else if(y == screen_height) {
				y--; show_sdn(0);
			}
			else {
				y = y < 0 ? 0 : screen_height-1;
				show_scr(0, screen_height-1);
			}
		}
		else if(flag[SHW] ) {
			show_scr(0, screen_height-1);
		}
		flag[SHW] = 0;

		show_pos();
		gotoxy(x-1,y+y2);
#ifndef TERMINAL
		draw_cursor();
		show_vbar();
#else
		lowvideo();
#endif /* TERMINAL */
	}
}

/* single char interpreter */
void main_exec(int key)
{
	cur_pos = get_cur();
	if(update_scr) {
#ifndef TERMINAL
		undraw_cursor();
#endif /* TERMINAL */
		if(flag[REC]) rec_macro(key);
	}
	if(help_done){
		help_done = 0;
		flag[SHW] = 1;
	} else if(literal) {
		key_normal(keve->state & ControlMask ? key & 0x1f : key);
		literal = 0;
	} else {
		if(key & 0xFF00) key_func(key); else
		if(keve->state & Mod1Mask) key_alt(key);
		else {
#ifdef GREEK
			/* Ctrl-C is enabled here by ^K^G */
			if(doCtrlC) ctrlc_key(key); else
#endif /* GREEK */
			if(doCtrlK) ctrlk_key(key); else
			if(doCtrlQ) ctrlq_key(key); else {
				if(keve->state & ControlMask) key &= 0x1f;
				if(key >= BLNK)  key_normal(key);
				else key_control(key);
			}
		}
	}
	if(!doCtrlK && !doCtrlQ && old_pos != line_start) {
		last_pos = old_pos;
		old_pos = line_start;
	}
	cur_pos = get_cur();

	if(update_scr) scr_update();
}
