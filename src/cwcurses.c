/*$T /cwcurses.c GC 1.150 2016-12-27 09:59:42 */

/*$I0 

    This file is part of CWStudio.

    Copyright 2008-2016 Lukasz Komsta, SP8QED

    CWStudio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CWStudio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CWStudio. If not, see <http://www.gnu.org/licenses/>.

 */
#include "cwstudio.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#define RANGE(x, l, u) \
	if(param.x < l) param.x = l; \
	if(param.x > u) param.x = u;
#define BOUND(x, l, u) \
	if(x < l) x = l; \
	if(x > u) x = u;
#if HAVE_WINDOWS_H
#include <windows.h>
#include "BladeMP3EncDLL.h"
#endif
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_WINHREADS
#include <process.h>
#endif
#ifdef HAVE_LIBWINMM
#define SOUND_INTERFACE "/waveout"
#elif defined HAVE_PULSEAUDIO
#define SOUND_INTERFACE "/pulseaudio"
#elif defined HAVE_SNDIO
#define SOUND_INTERFACE "/sndio"
#elif defined HAVE_AUDIOIO
#define SOUND_INTERFACE "/audioio"
#elif defined HAVE_OSS
#define SOUND_INTERFACE "/oss"
#elif defined HAVE_COREAUDIO
#define SOUND_INTERFACE "/coreaudio"
#else
#define SOUND_INTERFACE ""
#endif
#ifdef HAVE_PROCESS_H
#define THREAD_INTERFACE	""
#elif defined HAVE_PTHREAD
#define THREAD_INTERFACE	"/pthread"
#else
#define THREAD_INTERFACE	""
#endif
#if defined HAVE_NCURSESW_CURSES_H
#include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
#include <ncurses.h>
#elif defined HAVE_CURSES_H
#include <curses.h>
#endif
#ifdef __DJGPP__
#include <pc.h>
#include <time.h>
#endif

/* Global variables */
static char			*text = NULL, *morsetext = NULL;
static cw_sample	asound, csound;
static cw_param		param;
static int			filemode = 0, mode = 0, wordset = 100, chars;
static int			shouldgenerate = 1;
#ifdef __DJGPP__
static unsigned int bits = 8;
static unsigned int samplerate = 11025;
static char			sbconfig[32] = "";
static int			lpt_base = 0x378;
static int			dos_device = 0;
#define DOS_PCSPEAKER	0
#define DOS_LPT1		1
#define DOS_LPT2		2
#define DOS_SB8			3
#else
static unsigned int bits = 16;
static unsigned int samplerate = 44100;
#endif
static char			filename[256] = "", inputbuffer[256] = "";
static char			charset[256] = "abstgjnokqfmzixdrhewlypvcu8219376450?!/=";
static char			charset_backup[256] = "abstgjnokqfmzixdrhewlypvcu8219376450?!/=";
#ifdef HAVE_CURSES
volatile int		playmode = CWSTOPPED;
static char			statustext[256] = "Press <F1> or <1> for help.";
static WINDOW		*win_screen, *win_title, *win_param, *win_text, *win_help, *win_bar, *win_prompt;
#ifdef HAVE_MOUSEMASK
MEVENT				event;
#endif
#endif
#ifdef WIN32

/*$2- Center console window and change its title under WIN32 ---------------------------------------------------------*/

/* */
void cwstudio_initwinconsole()
{
	HWND	desktop = GetDesktopWindow();
	HWND	console = GetConsoleWindow();
	RECT	r, d;

	GetWindowRect(console, &r);
	GetWindowRect(desktop, &d);
	MoveWindow(console, (d.right - r.right) / 2, (d.bottom - r.bottom) / 2, r.right, r.bottom, TRUE);

	SetConsoleTitle("CWStudio");
}
#endif

/*$2- Write config file, name is taken from canonical host name ------------------------------------------------------*/

/* */
void cwstudio_writeconfig()
{
	FILE	*f;
	char	filename[255];
#ifdef WIN32
	sprintf(filename, "%s%s%s",getenv("HOMEDRIVE"),getenv("HOMEPATH"),"\\cwstudio.ini");
#elif defined __DJGPP__
	sprintf(filename, "%s","cwstudio.cfg");
#else
	char homedir[255];
	//if ((homedir = getenv("HOME")) == NULL) homedir = getpwuid(getuid())->pw_dir;
	sprintf(filename, "%s%s",getenv("HOME"),"/.cwstudio");
#endif
//	sprintf(filename, "%s", CANONICAL_HOST);
	if((f = fopen(filename, "w")) != NULL) {
		fprintf(f,"mode = %i\n",mode);
		fprintf(f,"wordset = %i\n",wordset);
		fprintf(f,"chars = %i\n",chars);
		fprintf(f,"bits = %i\n",bits);
		fprintf(f,"samplerate = %i\n",samplerate);
		fprintf(f,"agc = %i\n",param.agc);
		fprintf(f,"channels = %i\n",param.channels);
		fprintf(f,"click = %i\n",param.click);
		fprintf(f,"cspaces = %i\n",param.cspaces);
		fprintf(f,"dashlen = %i\n",param.dashlen);
		fprintf(f,"detune = %i\n",param.detune);
		fprintf(f,"even = %i\n",param.even);
		fprintf(f,"freq = %i\n",param.freq);
		fprintf(f,"hand = %i\n",param.hand);
		fprintf(f,"highcut = %i\n",param.highcut);
		fprintf(f,"hum = %i\n",param.hum);
		fprintf(f,"lowcut = %i\n",param.lowcut);
		fprintf(f,"noise = %i\n",param.noise);
		fprintf(f,"number = %i\n",param.number);
		fprintf(f,"odd = %i\n",param.odd);
		fprintf(f,"pan = %i\n",param.pan);
		fprintf(f,"pandrift = %i\n",param.pandrift);
		fprintf(f,"qsb = %i\n",param.qsb);
		fprintf(f,"seed = %i\n",param.seed);
		fprintf(f,"shape = %i\n",param.shape);
		fprintf(f,"signals = %i\n",param.signals);
		fprintf(f,"spacelen = %i\n",param.spacelen);
		fprintf(f,"sweep = %i\n",param.sweep);
		fprintf(f,"sweepness = %i\n",param.sweepness);
		fprintf(f,"tempo = %i\n",param.tempo);
		fprintf(f,"window = %i\n",param.window);
		fprintf(f,"wspaces = %i\n",param.wspaces);
		fprintf(f, "charset = %s\n", charset);
		/*fwrite(&mode, sizeof(int), 1, f);
		fwrite(&wordset, sizeof(int), 1, f);
		fwrite(&chars, sizeof(int), 1, f);
		fwrite(&bits, sizeof(int), 1, f);
		fwrite(&samplerate, sizeof(int), 1, f);
		fwrite(&param, sizeof(cw_param), 1, f);
		fprintf(f, "%s", charset); */
		fclose(f);
	}
}

/*$2- Read config file, name is taken from canonical host name -------------------------------------------------------*/

/* */
void cwstudio_readconfig()
{
	FILE	*f;
	int	i;
	char	filename[255], buffer[256];
#ifdef WIN32
	sprintf(filename, "%s%s%s",getenv("HOMEDRIVE"),getenv("HOMEPATH"),"\\cwstudio.ini");
#elif defined __DJGPP__
	sprintf(filename, "%s","cwstudio.cfg");
#else
	char homedir[255];
	//if ((homedir = getenv("HOME")) == NULL) homedir = getpwuid(getuid())->pw_dir;
	sprintf(filename, "%s%s",getenv("HOME"),"/.cwstudio");
#endif
	if((f = fopen(filename, "r")) != NULL) {
		while(fgets(buffer, 256, f) != NULL) {
		sscanf(buffer," mode = %i",&mode);
		sscanf(buffer," wordset = %i",&wordset);
		sscanf(buffer," chars = %i",&chars);
		sscanf(buffer," bits = %i",&bits);
		sscanf(buffer," samplerate = %i",&samplerate);
		sscanf(buffer," agc = %i",&param.agc);
		sscanf(buffer," channels = %i",&param.channels);
		sscanf(buffer," click = %i",&param.click);
		sscanf(buffer," cspaces = %i ",&param.cspaces);
		sscanf(buffer," dashlen = %i",&param.dashlen);
		sscanf(buffer," detune = %i",&param.detune);
		sscanf(buffer," even = %i",&param.even);
		sscanf(buffer," freq = %i",&param.freq);
		sscanf(buffer," hand = %i",&param.hand);
		sscanf(buffer," highcut = %i",&param.highcut);
		sscanf(buffer," hum = %i",&param.hum);
		sscanf(buffer," lowcut = %i",&param.lowcut);
		sscanf(buffer," noise = %i",&param.noise);
		sscanf(buffer," number = %i",&param.number);
		sscanf(buffer," odd = %i",&param.odd);
		sscanf(buffer," pan = %i",&param.pan);
		sscanf(buffer," pandrift = %i",&param.pandrift);
		sscanf(buffer," qsb = %i",&param.qsb);
		sscanf(buffer," seed = %i",&param.seed);
		sscanf(buffer," shape = %i",&param.shape);
		sscanf(buffer," signals = %i",&param.signals);
		sscanf(buffer," spacelen = %i",&param.spacelen);
		sscanf(buffer," sweep = %i",&param.sweep);
		sscanf(buffer," sweepness = %i",&param.sweepness);
		sscanf(buffer," tempo = %i",&param.tempo);
		sscanf(buffer," window = %i",&param.window);
		sscanf(buffer," wspaces = %i",&param.wspaces);
		sscanf(buffer," charset = %s",charset);
		}
		fclose(f);
		/* fread(&mode, sizeof(int), 1, f);
		fread(&wordset, sizeof(int), 1, f);
		fread(&chars, sizeof(int), 1, f);
		fread(&bits, sizeof(int), 1, f);
		fread(&samplerate, sizeof(int), 1, f);
		fread(&param, sizeof(cw_param), 1, f);
		fgets(charset, 256, f); */
		shouldgenerate = 1;
	}
}

/*
 =======================================================================================================================
    Text generation routine, separated into a function.
 =======================================================================================================================
 */
char *cwstudio_generate_text()
{
	/*~~~~~~~~~~~~~~~~~~~~~~*/
	char	*generated = NULL;

	/*~~~~~~~~~~~~~~~~~~~~~~*/
	switch(mode)
	{
	case 0:		generated = cw_rand_groups(param.number, param.shape, charset, param.seed); break;
	case 1:		generated = cw_rand_words(param.number, param.shape, wordset, param.seed); break;
	case 2:		generated = cw_rand_calls(param.number, param.shape, param.seed); break;
	default:	generated = NULL;
	}

	return(generated);
}

#ifdef HAVE_CURSES
static int	ncol, nrow;

/*
 =======================================================================================================================
    This function turns off curses mode, reinitializes it and recreates all windows. It is called in the beginning and
    (indirectly) after window/terminal resize. Each window is painted twice: one with larger dimensions to draw frames,
    the another as real window inside. No text is put inside windows besides program title.
 =======================================================================================================================
 */
void cwstudio_resetwindows()
{
#define SPLIT	ncol / 2 + 8
	endwin();
	refresh();
	win_screen = initscr();
	cbreak();
	noecho();
	curs_set(0);
	getmaxyx(stdscr, nrow, ncol);

	if(has_colors()) {
		start_color();
		init_pair(1, COLOR_RED, COLOR_WHITE);
		init_pair(2, COLOR_WHITE, COLOR_BLUE);
		init_pair(3, COLOR_BLACK, COLOR_WHITE);
		init_pair(4, COLOR_BLUE, COLOR_WHITE);
	}

	win_title = newwin(5, SPLIT, 0, 0);
	if(has_colors()) {
		wattron(win_title, COLOR_PAIR(1));
		wbkgd(win_title, COLOR_PAIR(1));
	}

	box(win_title, 0, 0);
	mvwprintw(win_title, 1, 1, "CWStudio %s (%ix%i)", VERSION, ncol, nrow);
#ifdef __DJGPP__
	switch(dos_device)
	{
	case DOS_SB8:		mvwprintw(win_title, 2, 1, "(%s%s)", CANONICAL_HOST, sbconfig); break;
	case DOS_PCSPEAKER: mvwprintw(win_title, 2, 1, "(%s/pcspeaker)", CANONICAL_HOST); break;
	case DOS_LPT1:		mvwprintw(win_title, 2, 1, "(%s/lpt0378h)", CANONICAL_HOST); break;
	case DOS_LPT2:		mvwprintw(win_title, 2, 1, "(%s/lpt0278h)", CANONICAL_HOST); break;
	}

#else
	mvwprintw(win_title, 2, 1, "(%s%s%s)", CANONICAL_HOST, SOUND_INTERFACE, THREAD_INTERFACE);
#endif
	mvwprintw(win_title, 3, 1, "(C) 2009-2016 Lukasz Komsta, SP8QED");
	wrefresh(win_title);

	win_param = newwin(nrow - 9, SPLIT, 5, 0);
	if(has_colors()) {
		wattron(win_param, COLOR_PAIR(2));
		wbkgd(win_param, COLOR_PAIR(2));
	}

	box(win_param, 0, 0);
	wrefresh(win_param);
	delwin(win_param);

	win_param = newwin(nrow - 11, SPLIT - 2, 6, 1);
	keypad(win_param, TRUE);
	if(has_colors()) {
		wattron(win_param, COLOR_PAIR(2));
		wbkgd(win_param, COLOR_PAIR(2));
	}

	keypad(win_param, TRUE);

	win_text = newwin(nrow - 4, ncol - (SPLIT), 0, SPLIT);
	if(has_colors()) {
		wattron(win_text, COLOR_PAIR(2));
		wbkgd(win_text, COLOR_PAIR(2));
	}

	box(win_text, 0, 0);
	wrefresh(win_text);
	delwin(win_text);

	win_text = newwin(nrow - 6, ncol - (SPLIT) - 2, 1, SPLIT + 1);
	if(has_colors()) {
		wattron(win_text, COLOR_PAIR(2));
		wbkgd(win_text, COLOR_PAIR(2));
	}

	keypad(win_text, TRUE);
	wrefresh(win_text);

	win_bar = newwin(4, ncol, nrow - 4, 0);
	if(has_colors()) {
		wattron(win_bar, COLOR_PAIR(4));
		wbkgd(win_bar, COLOR_PAIR(4));
	}

	keypad(win_bar, TRUE);
	wrefresh(win_bar);
#ifdef HAVE_MOUSEMASK
	mouseinterval(0);
#endif
}

/*
 =======================================================================================================================
    Repaint all texts inside windows and update status message at the bottom.
 =======================================================================================================================
 */
void cwstudio_repaintwindows()
{
	werase(win_param);

	wprintw(win_param, "* %i Hz / %i bits / %i ch", samplerate, bits,param.channels);
	if (param.channels > 1) wprintw(win_param, " / pan %i %+i\n",param.pan,param.pandrift); else wprintw(win_param, "\n"); 

	switch(mode)
	{
	case 0: wprintw(win_param, "* %s\n\n", charset); break;
	case 1: wprintw(win_param, "* %i words from %i most common\n\n", param.number, wordset); break;
	case 2: wprintw(win_param, "* %i calls\n\n", param.number); break;
	}

	wprintw(win_param, "* Frequency %i Hz\n", param.freq);
	if(param.even) wprintw(win_param, "* %i%% even harmonics ", param.even);
	if(param.odd) wprintw(win_param, "* %i%% odd harmonics ", param.odd);
	if(param.even || param.odd) wprintw(win_param, "\n");
	if(param.click) wprintw(win_param, "* %i dB Click ", param.click);
	if(param.hand) wprintw(win_param, "* Hand simulation %i%% ", param.hand);
	if(param.hum) wprintw(win_param, "* %i%% Hum ", param.hum);
	wprintw(win_param, "\n");
	if(param.sweepness) wprintw(win_param, "* Sweep from %i Hz, sweepness %i\n", param.sweep, param.sweepness);
	if(param.detune) wprintw(win_param, "* Detune %i%% ", param.detune);
	if(param.qsb) wprintw(win_param, "* QSB %i%% ", param.qsb);
	wprintw(win_param, "* Tempo is %i cpm ", param.tempo);
	if(param.cspaces) wprintw(win_param, "* Char spacing +%i ", param.cspaces);
	if(param.wspaces) wprintw(win_param, "* Word spacing +%i ", param.wspaces);
	wprintw(win_param, "\n");
	wprintw(win_param, "* Random seed: %i ", param.seed);
	wprintw(win_param, "* Random shape: %i \n", param.shape);
	if(param.signals > 1) wprintw(win_param, "* Mixing %i signals ", param.signals);
	wprintw(win_param, "\n");
	if(param.noise) {
		wprintw(win_param, "* Adding %i%% noise, %i-%i Hz ", param.noise, param.lowcut, param.highcut);
		if(param.agc) wprintw(win_param, "* %i%% AGC ", param.agc);
	}

	wprintw(win_param, "\n");
	if(param.dashlen != 300) wprintw(win_param, "* Dash length: %i%% ", param.dashlen);
	if(param.spacelen != 100) wprintw(win_param, "* Space length: %i%% ", param.spacelen);
	if((param.dashlen != 300) || (param.spacelen != 100)) wprintw(win_param, "\n");
	mvwprintw(win_param, nrow - 12, 0, "* %s", statustext);
	wrefresh(win_param);

	werase(win_text);
	wprintw(win_text, "%s", text);
	wrefresh(win_text);

	werase(win_bar);
	mvwprintw(win_bar, 0, 0, "[ Play ][ Stop ][Pause ][Random][ Mode ][ Freq ][Noise ][Reset ][ Help ][Shape ]");
	mvwprintw(win_bar, 1, 0, "[ AGC  ][Click ][ Dlen ][ SLen ][DetQSB][ Even ][ Odd  ][ Hand ][ Hum  ][Sweep ]");
#ifdef HAVE_WINDOWS_H
	mvwprintw(win_bar, 2, 0, "[ Rate ][ Bits ][ WAV  ][ MP3  ][ Copy ][Paste ]        [ Load ][<<< Groups >>>]");
#else
	mvwprintw(win_bar, 2, 0, "[ Rate ][ Bits ][ WAV  ]                                [ Load ][<<< Groups >>>]");
#endif
	mvwprintw(win_bar, 3, 0, "[<<< Tempo  >>>][<<<Signals >>>][<<<Wspaces >>>][<<<Cspaces >>>][<<<Charset >>>]");
	wrefresh(win_bar);

	if(shouldgenerate) cwstudio_writeconfig();
}

#ifdef HAVE_SIGNAL_H

/*
 =======================================================================================================================
    This function is called to repaint everything in changed terminal after SIGWINCH signal (terminal resize).
 =======================================================================================================================
 */
void cwstudio_resizeterm()
{
	cwstudio_resetwindows();
	cwstudio_repaintwindows();
}
#endif

/*
 =======================================================================================================================
    Paint help window and wait for any key.
 =======================================================================================================================
 */
void cwstudio_help()
{
	/*~~~~~~~~~~~*/
	int ncol, nrow;

	/*~~~~~~~~~~~*/
	getmaxyx(stdscr, nrow, ncol);

	win_help = newwin(20, 42, nrow / 2 - 12, ncol / 2 - 21);
	if(has_colors()) {
		wattron(win_help, COLOR_PAIR(3));
		wbkgd(win_help, COLOR_PAIR(3));
	}

	box(win_help, 0, 0);
	wrefresh(win_help);
	delwin(win_help);

	win_help = newwin(18, 40, nrow / 2 - 11, ncol / 2 - 20);
	if(has_colors()) {
		wattron(win_help, COLOR_PAIR(3));
		wbkgd(win_help, COLOR_PAIR(3));
	}

#ifdef HAVE_WINDOWS_H
	wprintw(win_help, "F1/1 - help, F2/2 - WAV, M - MP3\n");
#else
	wprintw(win_help, "F1/1 - help, F2/2 - save to WAV file\n");
#endif
	wprintw(win_help, "F3/3 - reset, #$ - pan, %^ - pandrift\n");
	wprintw(win_help, "F4/4 - regenerate random, SPACE - enter\n");
#ifdef HAVE_WINDOWS_H
	wprintw(win_help, "F5/5,ENTER-play, Ctrl-Ins/Del-copy,paste\n");
#else
	wprintw(win_help, "F5/5, ENTER - play\n");
#endif
	wprintw(win_help, "F6/6 - stop, F7/7 - pause\n");
	wprintw(win_help, "F8/8 - noise mode, F9/9 - freq\n");
	wprintw(win_help, "F11/- - detune/qsb, F12/= - mode\n");
	wprintw(win_help, "UP/DOWN(@!) - groups, ? - bits\n");
	wprintw(win_help, "LEFT(,)/RIGHT(.) - char spaces\n");
	wprintw(win_help, "Shift-LEFT(<)/RIGHT(>) - word spaces\n");
	wprintw(win_help, "HOME/END({}) - charset, / - rate\n");
	wprintw(win_help, "PGUP/PGDN/([]) - tempo, BKSP(\\) - shape\n");
	wprintw(win_help, "INS/DEL(:\") - signals, Q - hand\n");
	wprintw(win_help, "F10/0 - exit, S - sweep, C - click\n");
	wprintw(win_help, "A - AGC, E - even harmonics\n");
	wprintw(win_help, "H - hum, O - odd harmonics, () - chans\n");
	wprintw(win_help, "Shift-HOME(:) - dash length\n");
	wprintw(win_help, "Shift-END(') - space length\n");
	wrefresh(win_help);
	keypad(win_help, TRUE);

	wgetch(win_help);

	delwin(win_help);
	cwstudio_resetwindows();
}
#endif

/*
 =======================================================================================================================
    Input text string from user, with customized prompt and maximum size.
 =======================================================================================================================
 */
void cwstudio_input(const char *prompt, char *entered, int length)
{
	/*~~~~~~~~~~~*/
	int ncol, nrow;

	/*~~~~~~~~~~~*/
	getmaxyx(stdscr, nrow, ncol);

	win_prompt = newwin(4, ncol, nrow - 4, 0);
	if(has_colors()) {
		wattron(win_prompt, COLOR_PAIR(3));
		wbkgd(win_prompt, COLOR_PAIR(3));
	}

	box(win_prompt, 0, 0);
	mvwprintw(win_prompt, 0, 3, "%s", prompt);
	wrefresh(win_prompt);
	delwin(win_prompt);

	win_prompt = newwin(2, ncol - 2, nrow - 3, 1);
	if(has_colors()) {
		wattron(win_prompt, COLOR_PAIR(3));
		wbkgd(win_prompt, COLOR_PAIR(3));
	}

	wrefresh(win_prompt);
	echo();
	curs_set(2);
	mvwgetnstr(win_prompt, 0, 0, entered, length);
	noecho();
	curs_set(0);
	delwin(win_prompt);
}

/*
 =======================================================================================================================
    Free text memory and regenerate it.
 =======================================================================================================================
 */
int cwstudio_regeneratetext()
{
	if(!filemode) {
		cw_free(text);
		if((text = cwstudio_generate_text()) == NULL) return(CWALLOC);
		cw_free(morsetext);
		if((morsetext = cw_encode(text)) == NULL) return(CWALLOC);
	}

	return(CWOK);
}

/*
 =======================================================================================================================
    Regenerate sound.
 =======================================================================================================================
 */
int cwstudio_regeneratesound()
{
	int err;

	cw_freesample(&asound);
	cw_freesample(&csound);
	cw_initsample(&asound, NULL);
	asound.samplerate = samplerate;
	cw_initsample(&csound, &asound);
	wattron(win_text, COLOR_PAIR(1));
	wprintw(win_text, "\n\n *** Please wait *** \n");
	wattron(win_text, COLOR_PAIR(2));
	wrefresh(win_text);
	if((err = cw_signals(&asound, param, morsetext)) != CWOK) return(err);
	if((err = cw_convert(&asound, &csound, bits)) != CWOK) return(err);
	shouldgenerate = 0;
	return(CWOK);
}

/*
 =======================================================================================================================
    CWStudio - Main
 =======================================================================================================================
 */
int main(int argc, char **argv)
{
	/*~~~~~~~*/
	int					ch;
	int					i, err, m;
	FILE				*f;
	size_t				size;

#ifdef __DJGPP__
	uclock_t			start;
	long int			ii;
#endif
	const int			buttontable[40] =
	{
		'5',
		'6',
		'7',
		'4',
		'=',
		'9',
		'8',
		'3',
		'1',
		'\\',
		'A',
		'C',
		';',
		'\'',
		'-',
		'E',
		'O',
		'Q',
		'H',
		'S',
		'/',
		'?',
		'2',
#ifdef HAVE_WINDOWS_H
		'M',
		CTL_DEL,
		CTL_INS,
#else
		0,
		0,
		0,
#endif
		0,
		'L',
		'!',
		'@',
		'[',
		']',
		':',
		'"',
		'<',
		'>',
		',',
		'.',
		'{',
		'}'
	};
#ifdef HAVE_WINDOWS_H
	HINSTANCE			hDLL = NULL;
	BEINITSTREAM		beInitStream = NULL;
	BEENCODECHUNK		beEncodeChunk = NULL;
	BEDEINITSTREAM		beDeinitStream = NULL;
	BECLOSESTREAM		beCloseStream = NULL;
	BEVERSION			beVersion = NULL;
	BEWRITEVBRHEADER	beWriteVBRHeader = NULL;
	BE_ERR				error = 0;
	BE_CONFIG			beConfig = { 0, };
	FILE				*pFileOut = NULL;
	DWORD				dwSamples = 0;
	DWORD				dwMP3Buffer = 0;
	HBE_STREAM			hbeStream = 0;
	PBYTE				pMP3Buffer = NULL;
	PSHORT				pWAVBuffer = NULL;
	DWORD				dwRead = 0;
	DWORD				dwWrite = 0;
	DWORD				dwDone = 0;
	long int			length;

	HANDLE				h;
	HGLOBAL				hMem;
#endif

	/*~~~~~~~*/

	/* Initialize parameters */
	cw_initparam(&param); 
	cwstudio_readconfig(); 

#ifdef __DJGPP__
	cwstudio_sbinit(sbconfig);
	if(sb_base)
		dos_device = DOS_SB8;
	else
		dos_device = DOS_PCSPEAKER;
#endif

	/*$2- Parse command line argument (file name to read from --------------------------------------------------------*/

	filemode = 0;
	if(argc == 2) {
		if((f = fopen(argv[1], "r")) != NULL) {
			cw_free(text);
			text = cw_malloc(1024);
			size = 0;
			size = fread(text, 1, 1024, f);
			fclose(f);
			filemode = 1;
			shouldgenerate = 1;
			cw_free(morsetext);
			if((morsetext = cw_encode(text)) == NULL) return(CWALLOC);
			text[size] = '\0';
		}
	}

#ifdef WIN32
	cwstudio_initwinconsole();
#endif
#ifdef HAVE_WINDOWS_H
	hDLL = LoadLibrary("lame_enc.dll");
#endif
#if defined(HAVE_SIGNAL_H) && defined(SIGWINCH)
	/* Register signal to handle terminal resize */
	signal(SIGWINCH, cwstudio_resizeterm);
#endif
	chars = strlen(charset);

	/*$2- Initialize curses mode -------------------------------------------------------------------------------------*/

	initscr();
	cwstudio_resetwindows();
	cwstudio_regeneratetext();
	cwstudio_repaintwindows();

#ifdef HAVE_MOUSEMASK
	mousemask(BUTTON1_PRESSED, NULL);
#endif

	/*$3- Main loop for keyboard input in curses mode ================================================================*/

	while(((ch = wgetch(win_bar)) != KEY_F(10)) && (ch != '0'))
	{
#ifdef HAVE_MOUSEMASK
		if(ch == KEY_MOUSE)
		{
#ifdef HAVE_NC_GETMOUSE
			m = nc_getmouse(&event);
#else
			m = getmouse(&event);
#endif
			if(m == OK) {
				i = (event.y - nrow + 4) * 10 + (event.x / 8);
				ch = buttontable[i];
			}
		}
#endif
		switch(ch)
		{
		case KEY_F(1):
		case '1':
			cwstudio_help();
			break;

		case KEY_F(2):
		case '2':
			if(shouldgenerate) cwstudio_regeneratesound();
			i = (int) time(NULL);
			cwstudio_input("Filename without ext :", inputbuffer, 250);
			if(inputbuffer[0] == '\0')
				sprintf(filename, "%x.wav", i);
			else
				sprintf(filename, "%s.wav", inputbuffer);
			if((err = cw_wavout(filename, &csound)) != CWOK) return(i);
			if(inputbuffer[0] == '\0')
				sprintf(filename, "%x.txt", i);
			else
				sprintf(filename, "%s.txt", inputbuffer);
			f = fopen(filename, "w");
			fputs(text, f);
			fclose(f);
			if(inputbuffer[0] == '\0')
				sprintf(statustext, "%x.wav/txt saved.", i);
			else
				sprintf(statustext, "%s.wav/txt saved.", inputbuffer);
			break;

		case KEY_F(3):
		case '3':
			i = param.seed;
			cw_initparam(&param);
			param.seed = i;
			shouldgenerate = 1;
			break;

		case KEY_F(4):
		case '4':
			param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
			shouldgenerate = 1;
			filemode = 0;
			break;
#if defined(HAVE_OSS) || defined(HAVE_PULSEAUDIO) || defined(HAVE_LIBWINMM) || defined(HAVE_COREAUDIO) || defined(__DJGPP__)

		case KEY_F(5):
		case '5':
			if(shouldgenerate) cwstudio_regeneratesound();

#ifdef __DJGPP__
			if(dos_device == DOS_SB8)
			{
#endif
				playmode = cwstudio_play(&csound);
				if(playmode == CWPLAYING) strcpy(statustext, "Playback started.");
#ifdef __DJGPP__
			}
			else {
				strcpy(statustext, "Playback started.");
				cwstudio_repaintwindows();
				if(dos_device == DOS_PCSPEAKER) {
					outportb(0x43, 0xb6);
					outportb(0x42, 0xff);
					outportb(0x42, 0x00);
					outportb(0x43, 0x90);
					outportb(0x61, inportb(0x61) | 3);
				}

				for(ii = 0; ii < csound.length; ii++) {
					if(dos_device == DOS_PCSPEAKER)
						outportb(0x42, *(((unsigned char *) csound.data) + ii) >> 2);
					else
						outportb(lpt_base, *(((unsigned char *) csound.data) + ii));
					if(kbhit()) {
						switch(wgetch(win_bar))
						{
						case KEY_F(7):
							strcpy(statustext, "Playback paused.");
							cwstudio_repaintwindows();
							while(!kbhit());
							while(kbhit()) wgetch(win_bar);
							strcpy(statustext, "Playback resumed.");
							cwstudio_repaintwindows();
							break;

						case KEY_F(6):
							ii = csound.length;
							while(kbhit()) wgetch(win_bar);
							strcpy(statustext, "Playback stopped.");
							break;

						case KEY_F(5):
							ii = 0;
							while(kbhit()) wgetch(win_bar);
							break;
						}
					}

					start = uclock();
					while(uclock() < start + UCLOCKS_PER_SEC / 11025);
				}

				if(dos_device == DOS_PCSPEAKER) {
					outportb(0x43, 0xb6);
					outportb(0x61, inportb(0x61) & 0xfc);
				}
			}
#endif
			break;

		case KEY_F(6):
		case '6':
#ifdef __DJGPP__
			if(sb_base)
			{
#endif
				playmode = cwstudio_stop();
				strcpy(statustext, "Playback stopped.");
#ifdef __DJGPP__
			}
#endif
			break;

		case KEY_F(7):
		case '7':
#ifdef __DJGPP__
			if(sb_base)
			{
#endif
				playmode = cwstudio_pause();
				if(playmode == CWPLAYING)
					strcpy(statustext, "Playback resumed.");
				else if(playmode == CWPAUSED)
					strcpy(statustext, "Playback paused.");
				else if(playmode == CWSTOPPED)
					strcpy(statustext, "Playback stopped.");
#ifdef __DJGPP__
			}
#endif
			break;
#endif

		case KEY_F(8):
		case '8':
			if(param.noise == 100)
				param.noise = 0;
			else
				param.noise = param.noise + 25;
			if(param.lowcut == 300) {
				param.lowcut = 100;
				param.highcut = 6000;
			}
			else {
				param.lowcut = 300;
				param.highcut = 2400;
			}

			shouldgenerate = 1;
			break;

		case KEY_F(9):
		case '9':
			param.freq = param.freq + 100;
			if(param.freq > 4000) param.freq = 100;
			shouldgenerate = 1;
			break;

		case KEY_F(11):
		case '-':
			if(param.qsb == 0)
				param.qsb = 10;
			else if(param.qsb == 10)
				param.qsb = 0;

			if(param.detune >= 100)
				param.detune = 0;
			else
				param.detune = param.detune + 25;
			shouldgenerate = 1;
			break;

		case KEY_F(12):
		case '=':
			mode++;
			if(mode >= 3) mode = 0;
			shouldgenerate = 1;
			break;

		case KEY_BACKSPACE:
		case '\\':
			if(param.shape >= 20)
				param.shape = -20;
			else
				param.shape = param.shape + 5;
			if(!filemode) shouldgenerate = 1;
			break;

		case KEY_PPAGE:
		case ']':
			param.tempo = param.tempo + 5;
			RANGE(tempo, 5, 500);
			shouldgenerate = 1;
			break;

		case KEY_NPAGE:
		case '[':
			param.tempo = param.tempo - 5;
			RANGE(tempo, 5, 500);
			shouldgenerate = 1;
			break;

		case KEY_HOME:
		case '{':
			strncpy(charset, charset_backup, 256);
			chars--;
			BOUND(chars, 2, strlen(charset_backup));
			charset[chars] = '\0';
			if(!filemode) shouldgenerate = 1;
			break;

		case KEY_END:
		case '}':
			strncpy(charset, charset_backup, 256);
			chars++;
			BOUND(chars, 2, strlen(charset_backup));
			charset[chars] = '\0';
			if(!filemode) shouldgenerate = 1;
			break;

		case KEY_SHOME:
		case ';':
			if(param.dashlen >= 800)
				param.dashlen = 200;
			else
				param.dashlen = param.dashlen + 50;
			shouldgenerate = 1;
			break;

		case KEY_SEND:
		case '\'':
			if(param.spacelen >= 200)
				param.spacelen = 50;
			else
				param.spacelen = param.spacelen + 25;
			shouldgenerate = 1;
			break;

		case KEY_IC:
		case '"':
			param.signals++;
			RANGE(signals, 1, 5);
			shouldgenerate = 1;
			break;

		case KEY_DC:
		case ':':
			param.signals--;
			RANGE(signals, 1, 5);
			shouldgenerate = 1;
			break;

		case KEY_RIGHT:
		case '.':
			param.cspaces++;
			RANGE(cspaces, 0, 100);
			shouldgenerate = 1;
			break;

		case KEY_LEFT:
		case ',':
			param.cspaces--;
			RANGE(cspaces, 0, 100);
			shouldgenerate = 1;
			break;

		case ')':
			param.channels++;
			RANGE(channels, 1, 7);
			shouldgenerate = 1;
			break;

		case '(':
			param.channels--;
			RANGE(channels, 1, 7);
			shouldgenerate = 1;
			break;

		case KEY_SRIGHT:
		case '>':
			param.wspaces++;
			RANGE(wspaces, 0, 100);
			shouldgenerate = 1;
			break;

		case KEY_SLEFT:
		case '<':
			param.wspaces--;
			RANGE(wspaces, 0, 100);
			shouldgenerate = 1;
			break;

		case KEY_UP:
		case '!':
			param.number = param.number - 5;
			RANGE(number, 5, 100);
			if(!filemode) shouldgenerate = 1;
			break;

		case KEY_DOWN:
		case '@':
			param.number = param.number + 5;
			RANGE(number, 5, 100);
			if(!filemode) shouldgenerate = 1;
			break;

		case '#':
			param.pan = param.pan - 30;
			RANGE(pan, -720, 720);
			if(!filemode) shouldgenerate = 1;
			break;

		case '$':
			param.pan = param.pan + 30;
			RANGE(pan, -720, 720);
			if(!filemode) shouldgenerate = 1;
			break;

		case '%':
			param.pandrift = param.pandrift - 1;
			RANGE(pandrift, -180, 180);
			if(!filemode) shouldgenerate = 1;
			break;

		case '^':
			param.pandrift = param.pandrift + 1;
			RANGE(pandrift, -180, 180);
			if(!filemode) shouldgenerate = 1;
			break;


#ifndef __DJGPP__
		case '/':
			if(samplerate == 8000)
				samplerate = 11025;
			else if(samplerate == 11025)
				samplerate = 22050;
			else if(samplerate == 22050)
				samplerate = 44100;
			else if(samplerate == 44100)
				samplerate = 16000;
			else if(samplerate == 16000)
				samplerate = 24000;
			else if(samplerate == 24000)
				samplerate = 48000;
			else if(samplerate == 48000)
				samplerate = 96000;
			else if(samplerate == 96000)
				samplerate = 192000;
			else if(samplerate == 192000)
				samplerate = 8000;
			shouldgenerate = 1;
			break;

		case '?':
			if(bits == 16)
				bits = 8;
			else
				bits = 16;
			shouldgenerate = 1;
			break;

#endif

		case 'A':
		case 'a':
			if(param.agc >= 100)
				param.agc = 0;
			else
				param.agc = param.agc + 25;
			shouldgenerate = 1;
			break;

		case 'C':
		case 'c':
			if(param.click >= 10)
				param.click = 1;
			else
				param.click = param.click + 2;
			shouldgenerate = 1;
			break;

		case 'E':
		case 'e':
			if(param.even >= 10)
				param.even = 0;
			else
				param.even++;
			shouldgenerate = 1;
			break;

		case 'H':
		case 'h':
			if(param.hum >= 100)
				param.hum = 0;
			else
				param.hum = param.hum + 25;
			shouldgenerate = 1;
			break;

		case 'L':
		case 'l':
			cwstudio_input("Filename :", filename, 250);
			filemode = 0;
			if(strcmp(filename, "")) {
				cw_free(text);
				text = cw_malloc(1024);
				size = 0;
				if((f = fopen(filename, "r")) != NULL) {
					size = fread(text, 1, 1024, f);
					fclose(f);
					filemode = 1;
					shouldgenerate = 1;
					cw_free(morsetext);
					if((morsetext = cw_encode(text)) == NULL) return(CWALLOC);
				}
				else
					sprintf(statustext, "No such file.");
				text[size] = '\0';
			}
			break;

		case ' ':
			cwstudio_input("Text :", inputbuffer, 255);
			if(strcmp(inputbuffer, "")) {
				cw_free(text);
				text = cw_malloc(1024);
				strncpy(text, inputbuffer, 255);
				filemode = 1;
				shouldgenerate = 1;
				cw_free(morsetext);
				if((morsetext = cw_encode(text)) == NULL) return(CWALLOC);
			}
			break;

		case 'O':
		case 'o':
			if(param.odd >= 10)
				param.odd = 0;
			else
				param.odd++;
			shouldgenerate = 1;
			break;

		case 's':
		case 'S':
			if(param.sweepness == 200)
				param.sweepness = 0;
			else
				param.sweepness = 200;

			if(param.sweep >= 3000)
				param.sweep = -3000;
			else
				param.sweep = param.sweep + 1000;
			shouldgenerate = 1;
			break;

		case 'Q':
		case 'q':
			if(param.hand >= 100)
				param.hand = 0;
			else
				param.hand = param.hand + 20;
			shouldgenerate = 1;
			break;
#ifdef __DJGPP__

		case '`':
			dos_device++;
			dos_device &= 3;
			if(!sb_base & (dos_device == 3)) dos_device = 0;
			if(dos_device == DOS_LPT1) lpt_base = 0x378;
			if(dos_device == DOS_LPT2) lpt_base = 0x278;
			cwstudio_resetwindows();
			break;
#endif
#ifdef HAVE_WINDOWS_H

		case CTL_INS:
			OpenClipboard(NULL);
			h = GetClipboardData(CF_TEXT);
			if(strcmp((char *) h, "")) {
				cw_free(text);
				text = cw_malloc(1024);
				i = -1;
				strncpy(text, (char *) h, 255);
				for(i = 0; i < 256; i++)
					if(text[i] == '\r') text[i] = ' ';
				filemode = 1;
				shouldgenerate = 1;
				cw_free(morsetext);
				if((morsetext = cw_encode(text)) == NULL) return(CWALLOC);
			}

			CloseClipboard();
			break;

		case CTL_DEL:
			hMem = GlobalAlloc(GMEM_MOVEABLE, 2 * strlen(text) + 1);
			f = fopen("cw.tmp", "w");
			fputs(text, f);
			fclose(f);
			f = fopen("cw.tmp", "rb");
			fread(GlobalLock(hMem), 1, 2 * strlen(text), f);
			fclose(f);
			remove("cw.tmp");
			GlobalUnlock(hMem);
			OpenClipboard(0);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, hMem);
			CloseClipboard();
			break;

		case 'M':
		case 'm':
			if(hDLL == NULL)
				sprintf(statustext, "No lame_enc.dll found.");
			else {
				beInitStream = (BEINITSTREAM) GetProcAddress(hDLL, TEXT_BEINITSTREAM);
				beEncodeChunk = (BEENCODECHUNK) GetProcAddress(hDLL, TEXT_BEENCODECHUNK);
				beDeinitStream = (BEDEINITSTREAM) GetProcAddress(hDLL, TEXT_BEDEINITSTREAM);
				beCloseStream = (BECLOSESTREAM) GetProcAddress(hDLL, TEXT_BECLOSESTREAM);
				beVersion = (BEVERSION) GetProcAddress(hDLL, TEXT_BEVERSION);
				beWriteVBRHeader = (BEWRITEVBRHEADER) GetProcAddress(hDLL, TEXT_BEWRITEVBRHEADER);

				if
				(
					!beInitStream
				||	!beEncodeChunk
				||	!beDeinitStream
				||	!beCloseStream
				||	!beVersion
				||	!beWriteVBRHeader
				) sprintf(statustext, "lame_enc.dll error.");
				else if((bits != 16) || (samplerate != 44100))
					sprintf(statustext, "Unsupported samplerate/bits.");
				else {
					if(shouldgenerate) {
						cw_freesample(&asound);
						cw_freesample(&csound);
						cw_initsample(&asound, NULL);
						asound.samplerate = samplerate;
						cw_initsample(&csound, &asound);
						wattron(win_text, COLOR_PAIR(1));
						wprintw(win_text, "\n\n *** Please wait *** \n");
						wattron(win_text, COLOR_PAIR(2));
						wrefresh(win_text);
						if((err = cw_signals(&asound, param, morsetext)) != CWOK) return(err);
						if((err = cw_convert(&asound, &csound, bits)) != CWOK) return(err);
						shouldgenerate = 0;
					}

					i = (int) time(NULL);
					cwstudio_input("Filename without ext :", inputbuffer, 8);
					if(inputbuffer[0] == '\0')
						sprintf(filename, "%x.mp3", i);
					else
						sprintf(filename, "%s.mp3", inputbuffer);
					pFileOut = fopen(filename, "wb+");
					memset(&beConfig, 0, sizeof(beConfig));

					beConfig.dwConfig = BE_CONFIG_LAME;
					beConfig.format.LHV1.dwStructVersion = 1;
					beConfig.format.LHV1.dwStructSize = sizeof(beConfig);
					beConfig.format.LHV1.dwSampleRate = csound.samplerate;
					beConfig.format.LHV1.dwReSampleRate = 0;
					beConfig.format.LHV1.nMode = BE_MP3_MODE_MONO;
					beConfig.format.LHV1.dwBitrate = 128;
					beConfig.format.LHV1.dwMaxBitrate = 320;
					beConfig.format.LHV1.nPreset = LQP_NOPRESET;
					beConfig.format.LHV1.dwMpegVersion = MPEG1;
					beConfig.format.LHV1.dwPsyModel = 0;
					beConfig.format.LHV1.dwEmphasis = 0;
					beConfig.format.LHV1.bOriginal = TRUE;
					beConfig.format.LHV1.bCRC = TRUE;
					beConfig.format.LHV1.bCopyright = TRUE;
					beConfig.format.LHV1.bPrivate = TRUE;
					beConfig.format.LHV1.bWriteVBRHeader = FALSE;
					beConfig.format.LHV1.bEnableVBR = FALSE;
					beConfig.format.LHV1.nVBRQuality = 5;
					beConfig.format.LHV1.bNoRes = TRUE;

					error = beInitStream(&beConfig, &dwSamples, &dwMP3Buffer, &hbeStream);
					pMP3Buffer = malloc(dwMP3Buffer);

					pWAVBuffer = (PSHORT) csound.data;

					length = csound.length;
					wattron(win_text, COLOR_PAIR(1));
					wprintw(win_text, "\n\nMP3 Exporting...\n\n");
					wattron(win_text, COLOR_PAIR(2));
					wrefresh(win_text);

					while(length >= dwSamples) {
						error = beEncodeChunk(hbeStream, dwSamples, pWAVBuffer, pMP3Buffer, &dwWrite);
						fwrite(pMP3Buffer, 1, dwWrite, pFileOut);
						dwDone += dwRead * sizeof(SHORT);

						length -= dwSamples;
						pWAVBuffer += dwSamples;
					}

					if(length > 0) {
						error = beEncodeChunk(hbeStream, length, pWAVBuffer, pMP3Buffer, &dwWrite);
						fwrite(pMP3Buffer, 1, dwWrite, pFileOut);
					}

					error = beDeinitStream(hbeStream, pMP3Buffer, &dwWrite);
					if(dwWrite) fwrite(pMP3Buffer, 1, dwWrite, pFileOut);

					beCloseStream(hbeStream);
					free(pMP3Buffer);
					fclose(pFileOut);

					if(inputbuffer[0] == '\0')
						sprintf(filename, "%x.txt", i);
					else
						sprintf(filename, "%s.txt", inputbuffer);
					f = fopen(filename, "w");
					fputs(text, f);
					fclose(f);

					if(inputbuffer[0] == '\0')
						sprintf(statustext, "%x.mp3/txt saved.", i);
					else
						sprintf(statustext, "%s.mp3/txt saved.", inputbuffer);
				}
			}
			break;
#endif
		}

		/*$2- Regenerate text and refresh screen after each keypress -------------------------------------------------*/

		cwstudio_regeneratetext();
		cwstudio_repaintwindows();
	}
#ifdef __DJGPP__
	if(sb_base)
#endif
#if defined(HAVE_OSS) || defined(HAVE_PULSEAUDIO) || defined(HAVE_LIBWINMM) || defined(HAVE_COREAUDIO) || defined(__DJGPP__)
	cwstudio_stop();
#endif

	/* End curses */
	endwin();
	initscr();
	refresh();
	endwin();

	/* Free memory */
	cw_freesample(&asound);
	cw_freesample(&csound);
	cw_free(text);
	cw_free(morsetext);

#ifdef HAVE_WINDOWS_H
	FreeLibrary(hDLL);
#endif

	/* Exit */
	return(CWOK);
}
