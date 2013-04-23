/*$T src/cwstudio.c GC 1.140 04/22/13 17:07:17 */

/*$I0

    This file is part of CWStudio.

    Copyright 2008-2013 Lukasz Komsta, SP8QED

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
#elif defined HAVE_OSS
#define SOUND_INTERFACE "/oss"
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
#elif defined HAVE_TERMIOS_H
#include <termios.h>

/*
 =======================================================================================================================
    This is getch() replacement for compilation without ncurses, used for control of playback. The aim is to turn off
    echo in terminal and do not wait for ENTER. In the case of WIN32, the _getch() of conio.h is used. Any other
    configuration will result in getchar() behavior (echo and waiting for ENTER).
 =======================================================================================================================
 */
int getch()
{
	/*~~~~~~~~~~~~~~~~~~~~~~~*/
	struct termios	oldt, newt;
	int				ch;
	/*~~~~~~~~~~~~~~~~~~~~~~~*/

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

#elif defined WIN32
#include <conio.h>
#define getch	_getch
#else
#define getch	getchar
#endif

/*
 =======================================================================================================================
    Separate arguments form environmental variable to create argv-like table. Function based on "C FAQ" book by Steve
    Summit.
 =======================================================================================================================
 */
int separg(char *options, char *argv[], int size)
{
	/*~~~~~~~~~~~~~~~~~~~*/
	char	*str = options;
	int		i, argc = 0;
	/*~~~~~~~~~~~~~~~~~~~*/

	for(i = 0; i < size; i++) {
		while(isspace((int) *str)) str++;
		if(*str != '\0')
			argv[argc++] = str;
		else {
			argv[argc] = 0;
			break;
		}

		while((*str != '\0') && !isspace((int) *str)) str++;
		if((*str != '\0') && i < (size - 1)) *str++ = '\0';
	}

	return(argc);
}

/* Global variables */
static char			*text = NULL, *morsetext = NULL;
static cw_sample	asound, csound;
static cw_param		param;
static int			play = 1, output = 1, mode = 0, wordset = 100, chars;
static unsigned int bits = 16;
static unsigned int samplerate = 44100;
static char			filename[256] = "output.wav";
static char			charset[256] = "abstgjnokqfmzixdrhewlypvcu8219376450?!/=";
static char			charset_backup[256] = "abstgjnokqfmzixdrhewlypvcu8219376450?!/=";
#ifdef HAVE_CURSES
static int			playmode = CWSTOPPED;
static char			statustext[256] = "Press <F1> or <1> for help.";
static WINDOW		*win_title, *win_param, *win_text, *win_help;
#ifdef HAVE_CURSES_MOUSE
MEVENT				event;
#endif
#endif

/*
 =======================================================================================================================
    Parse environmental and command line parameters
 =======================================================================================================================
 */
void cwstudio_parseparam(int argc, char **argv)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	char	*envv[20], *envs, *totalv[256];
	int		envc = 0, i, c, val;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* If environmental variable is set, parse it */
	if((envs = getenv("CWPARAM")) != NULL) {
		envc = separg(envs, envv, 20);
		if(envc > 0) {
			totalv[0] = argv[0];
			for(i = 0; i < envc; i++) totalv[i + 1] = envv[i];
			for(i = envc; i < (envc + argc); i++) totalv[i + 1] = argv[i - envc + 1];
		}
	}

	/* else copy argv only */
	else
		for(i = 0; i < argc; i++) totalv[i] = argv[i];

	/* Parse command line parameters */
	while(1) {

		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		int						option_index = 0;
		static struct option	long_options[] =
		{
			{ "play", no_argument, &play, 1 },
			{ "noplay", no_argument, &play, 0 },
			{ "silent", no_argument, &output, 0 },
			{ "verbose", no_argument, &output, 1 },
			{ "groups", no_argument, &mode, 0 },
			{ "words", no_argument, &mode, 1 },
			{ "calls", no_argument, &mode, 2 },
			{ "agc", required_argument, NULL, 'a' },
			{ "bits", required_argument, NULL, 'b' },
			{ "click", required_argument, NULL, 'l' },
			{ "chars", required_argument, NULL, 'c' },
			{ "charset", required_argument, NULL, 'C' },
			{ "cspaces", required_argument, NULL, 's' },
			{ "dashlen", required_argument, NULL, 'D' },
			{ "detune", required_argument, NULL, 'd' },
			{ "even", required_argument, NULL, 'E' },
			{ "freq", required_argument, NULL, 'f' },
			{ "number", required_argument, NULL, 'n' },
			{ "hand", required_argument, NULL, 'H' },
			{ "highcut", required_argument, NULL, 'y' },
			{ "hum", required_argument, NULL, 'h' },
			{ "lowcut", required_argument, NULL, 'x' },
			{ "noise", required_argument, NULL, 'N' },
			{ "odd", required_argument, NULL, 'O' },
			{ "output", required_argument, NULL, 'o' },
			{ "qsb", required_argument, NULL, 'q' },
			{ "samplerate", required_argument, NULL, 'B' },
			{ "seed", required_argument, NULL, 'r' },
			{ "shape", required_argument, NULL, 'X' },
			{ "signals", required_argument, NULL, 'S' },
			{ "spacelen", required_argument, NULL, 'L' },
			{ "sweep", required_argument, NULL, 'p' },
			{ "sweepness", required_argument, NULL, 'P' },
			{ "tempo", required_argument, NULL, 't' },
			{ "window", required_argument, NULL, 'w' },
			{ "wordset", required_argument, NULL, 'v' },
			{ "wspaces", required_argument, NULL, 'W' },
			{ 0, 0, 0, 0 }
		};
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

		c = getopt_long_only
			(
				argc + envc,
				totalv,
				"a:b:B:c:C:d:D:E:f:g:h:H:l:L:n:o:O:p:P:q:r:s:S:t:v:w:W:x:X:y:",
				long_options,
				&option_index
			);
		if(c == -1) break;
		switch(c)
		{
		case 'a':
			param.agc = atoi(optarg);
			RANGE(agc, 0, 100);
			break;

		case 'b':
			bits = atoi(optarg);
			if((bits != 8) && (bits != 16)) bits = 16;
			break;

		case 'B':
			samplerate = atoi(optarg);
			if
			(
				(samplerate != 8000)
			&&	(samplerate != 11025)
			&&	(samplerate != 22050)
			&&	(samplerate != 44100)
			&&	(samplerate != 16000)
			&&	(samplerate != 24000)
			&&	(samplerate != 48000)
			&&	(samplerate != 96000)
			&&	(samplerate != 192000)
			) samplerate = 44100;
			break;

		case 'c':
			chars = atoi(optarg);
			if((chars > 0) && (chars < 256)) charset[chars] = '\0';
			break;

		case 'C':
			strncpy(charset, optarg, 256);
			strncpy(charset_backup, optarg, 256);
			break;

		case 'd':
			param.detune = atoi(optarg);
			RANGE(detune, 0, 100);
			break;

		case 'D':
			param.dashlen = atoi(optarg);
			if(param.dashlen == 0) param.dashlen = 300;
			RANGE(dashlen, 100, 10000);
			break;

		case 'E':
			param.even = atoi(optarg);
			RANGE(even, 0, 100);
			break;

		case 'f':
			param.freq = atoi(optarg);
			RANGE(freq, 50, 4000);
			break;

		case 'n':
			param.number = atoi(optarg);
			RANGE(number, 1, 100);
			break;

		case 'h':
			param.hum = atoi(optarg);
			RANGE(hum, 0, 100);
			break;

		case 'H':
			param.hand = atoi(optarg);
			RANGE(hand, 0, 100);
			break;

		case 'l':
			param.click = atoi(optarg);
			RANGE(click, 0, 100);
			break;

		case 'L':
			param.spacelen = atoi(optarg);
			if(param.spacelen == 0) param.spacelen = 100;
			RANGE(spacelen, 20, 300);
			break;

		case 'N':
			param.noise = atoi(optarg);
			RANGE(noise, 0, 100);
			break;

		case 'o':
			strncpy(filename, optarg, 256);
			break;

		case 'O':
			param.odd = atoi(optarg);
			RANGE(odd, 0, 100);
			break;

		case 'p':
			param.sweep = atoi(optarg);
			RANGE(sweep, -4000, 4000);
			break;

		case 'P':
			param.sweepness = atoi(optarg);
			RANGE(sweepness, 0, 10000);
			break;

		case 'q':
			param.qsb = atoi(optarg);
			RANGE(qsb, 0, 100);
			break;

		case 'r':
			if((val = atoi(optarg))) param.seed = val;
			RANGE(seed, 0, 32767);
			break;

		case 's':
			param.cspaces = atoi(optarg);
			RANGE(cspaces, 0, 100);
			break;

		case 'S':
			param.signals = atoi(optarg);
			RANGE(signals, 1, 5);
			break;

		case 't':
			param.tempo = atoi(optarg);
			RANGE(tempo, 5, 500);
			break;

		case 'v':
			wordset = atoi(optarg);
			if(wordset < 10) wordset = 10;
			if(wordset > 1000) wordset = 1000;
			break;

		case 'w':
			param.window = atoi(optarg);
			RANGE(window, 0, 1000);
			break;

		case 'W':
			param.wspaces = atoi(optarg);
			RANGE(wspaces, 0, 100);
			break;

		case 'x':
			param.lowcut = atoi(optarg);
			RANGE(lowcut, 50, 1000);
			break;

		case 'X':
			param.shape = atoi(optarg);
			RANGE(shape, -50, 50);
			break;

		case 'y':
			param.highcut = atoi(optarg);
			RANGE(highcut, 300, 10000);
			break;
		}
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
	case 0: generated = cw_rand_groups(param.number, param.shape, charset, param.seed); break;
	case 1: generated = cw_rand_words(param.number, param.shape, wordset, param.seed); break;
	case 2: generated = cw_rand_calls(param.number, param.shape, param.seed); break;
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
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	getmaxyx(stdscr, nrow, ncol);

	if(has_colors()) {
		start_color();
		init_pair(1, COLOR_WHITE, COLOR_RED);
		init_pair(2, COLOR_WHITE, COLOR_BLUE);
		init_pair(3, COLOR_BLACK, COLOR_WHITE);
	}

	win_title = newwin(5, SPLIT, 0, 0);
	if(has_colors()) {
		wattron(win_title, COLOR_PAIR(1));
		wbkgd(win_title, COLOR_PAIR(1));
	}

	box(win_title, 0, 0);
	mvwprintw(win_title, 1, 1, "CWStudio %s (%ix%i)", VERSION, ncol, nrow);
	mvwprintw(win_title, 2, 1, "(%s%s%s)", CANONICAL_HOST, SOUND_INTERFACE, THREAD_INTERFACE);
	mvwprintw(win_title, 3, 1, "(C) 2009-2013 Lukasz Komsta, SP8QED");
	wrefresh(win_title);

	win_param = newwin(nrow - 5, SPLIT, 5, 0);
	if(has_colors()) {
		wattron(win_param, COLOR_PAIR(2));
		wbkgd(win_param, COLOR_PAIR(2));
	}

	box(win_param, 0, 0);
	wrefresh(win_param);
	delwin(win_param);

	win_param = newwin(nrow - 7, SPLIT - 2, 6, 1);
	keypad(win_param, TRUE);
	if(has_colors()) {
		wattron(win_param, COLOR_PAIR(2));
		wbkgd(win_param, COLOR_PAIR(2));
	}

	keypad(win_param, TRUE);

	win_text = newwin(nrow, ncol - (SPLIT), 0, SPLIT);
	if(has_colors()) {
		wattron(win_text, COLOR_PAIR(2));
		wbkgd(win_text, COLOR_PAIR(2));
	}

	box(win_text, 0, 0);
	wrefresh(win_text);
	delwin(win_text);

	win_text = newwin(nrow - 2, ncol - (SPLIT) - 2, 1, SPLIT + 1);
	if(has_colors()) {
		wattron(win_text, COLOR_PAIR(2));
		wbkgd(win_text, COLOR_PAIR(2));
	}

	keypad(win_text, TRUE);
	wrefresh(win_text);
}

/*
 =======================================================================================================================
    Repaint all texts inside windows and update status message at the bottom.
 =======================================================================================================================
 */
void cwstudio_repaintwindows()
{
	werase(win_param);

	wprintw(win_param, "* %i Hz / %i bits\n", samplerate, bits);

	if(isatty(STDIN_FILENO)) {
		switch(mode)
		{
		case 0: wprintw(win_param, "* %s\n\n", charset); break;
		case 1: wprintw(win_param, "* %i words from %i most common\n\n", param.number, wordset); break;
		case 2: wprintw(win_param, "* %i calls\n\n", param.number); break;
		}
	}
	else {
		wprintw(win_param, "* Getting text from stdin\n\n");
	}

	wprintw(win_param, "* Frequency %i Hz * Window %i samples\n", param.freq, param.window);
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
	wprintw(win_param, "\n");
	wprintw(win_param, "* Tempo is %i wpm ", param.tempo);
	if(param.cspaces) wprintw(win_param, "* Char spacing +%i ", param.cspaces);
	if(param.wspaces) wprintw(win_param, "* Word spacing +%i ", param.wspaces);
	wprintw(win_param, "\n");
	wprintw(win_param, "* Random seed: %i ", param.seed);
	if(param.shape)
		wprintw(win_param, "* Random shape: %i \n", param.shape);
	else
		wprintw(win_param, "\n");
	if(param.signals > 1) wprintw(win_param, "* Mixing %i signals ", param.signals);
	wprintw(win_param, "\n");
	if(param.noise) {
		wprintw(win_param, "* Adding %i%% noise, %i - %i Hz ", param.noise, param.lowcut, param.highcut);
		if(param.agc) wprintw(win_param, "* %i%% AGC ", param.agc);
		wprintw(win_param, "\n");
	}

	if(param.dashlen != 300) wprintw(win_param, "* Dash length: %i%% ", param.dashlen);
	if(param.spacelen != 100) wprintw(win_param, "* Space length: %i%% ", param.spacelen);
	if((param.dashlen != 300) || (param.spacelen != 100)) wprintw(win_param, "\n");
	wrefresh(win_param);

	werase(win_text);
	wprintw(win_text, "%s", text);
	wrefresh(win_text);
	mvwprintw(win_param, nrow - 8, 0, "* %s", statustext);
	wrefresh(win_param);
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

	win_help = newwin(20, 40, nrow / 2 - 10, ncol / 2 - 20);
	if(has_colors()) {
		wattron(win_help, COLOR_PAIR(3));
		wbkgd(win_help, COLOR_PAIR(3));
	}

	box(win_help, 0, 0);
	wrefresh(win_help);
	delwin(win_help);

	win_help = newwin(18, 38, nrow / 2 - 9, ncol / 2 - 19);
	if(has_colors()) {
		wattron(win_help, COLOR_PAIR(3));
		wbkgd(win_help, COLOR_PAIR(3));
	}

	wprintw(win_help, "F1 - help, F2 - save to WAV file\n");
	wprintw(win_help, "F3 - reset parameters\n");
	wprintw(win_help, "F4, SPACE - regenerate random\n");
	wprintw(win_help, "F5, ENTER - play\n");
	wprintw(win_help, "F6 - stop, F7 - pause\n");
	wprintw(win_help, "F8 - noise mode, F9/F10 - freq\n");
	wprintw(win_help, "F11 - detune/qsb, F12 - mode\n");
	wprintw(win_help, "UP/DOWN - groups, ? - bits\n");
	wprintw(win_help, "LEFT/RIGHT - char spaces\n");
	wprintw(win_help, "Shift-LEFT/RIGHT - word spaces\n");
	wprintw(win_help, "HOME/END - charset, / - samplerate\n");
	wprintw(win_help, "PGUP/PGDN - tempo, BACKSPACE - shape\n");
	wprintw(win_help, "INS/DEL - signals, Q - hand\n");
	wprintw(win_help, "Ctrl-C - exit, S - sweep\n");
	wprintw(win_help, "A - AGC, E - even harmonics\n");
	wprintw(win_help, "H - hum, O - odd harmonics\n");
	wprintw(win_help, "Shift-HOME - dash length\n");
	wprintw(win_help, "Shift-END - space length\n");
	wrefresh(win_help);
	keypad(win_help, TRUE);

	wgetch(win_help);

	delwin(win_help);
	cwstudio_resetwindows();
}
#endif

/*
 =======================================================================================================================
    Free text memory and regenerate it.
 =======================================================================================================================
 */
int cwstudio_regeneratetext()
{
	cw_free(text);
	cw_free(morsetext);
	if((text = cwstudio_generate_text()) == NULL) return(CWALLOC);
	if((morsetext = cw_encode(text)) == NULL) return(CWALLOC);
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
	int ch;
	int i, err;
	FILE *f;
	/*~~~~~~~*/

	/* Initialize parameters */
	cw_initparam(&param);

	/* Parse environmental and command line parameters */
	cwstudio_parseparam(argc, argv);

#ifdef WIN32
	SetConsoleTitle("CWStudio");
#endif
#ifdef HAVE_CURSES
	if(play)
	{
#if defined(HAVE_SIGNAL_H) && defined(SIGWINCH)
		/* Register signal to handle terminal resize */
		signal(SIGWINCH, cwstudio_resizeterm);
#endif
		chars = strlen(charset);

		/*$2- Initialize curses mode ---------------------------------------------------------------------------------*/

		initscr();
		cwstudio_resetwindows();
		cwstudio_regeneratetext();
		cwstudio_repaintwindows();

#ifdef HAVE_CURSES_MOUSE
		mousemask(ALL_MOUSE_EVENTS, NULL);
#endif

		/*$3- Main loop for keyboard input in curses mode ============================================================*/

		while((ch = wgetch(win_param))) {
			switch(ch)
			{
#ifdef HAVE_CURSES_MOUSE

			/* Mouse suppord compiled conditionally */
			case KEY_MOUSE:
				if(getmouse(&event) == OK) {
					if(event.bstate & BUTTON1_PRESSED) {
						if((playmode == CWPLAYING) || (playmode == CWPAUSED)) {
							playmode = cwstudio_stop();
							strcpy(statustext, "Playback stopped.");
						}
						else {
							cw_freesample(&asound);
							cw_freesample(&csound);
							cw_initsample(&asound, NULL);
							asound.samplerate = samplerate;
							cw_initsample(&csound, &asound);
							if((err = cw_signals(&asound, param, morsetext)) != CWOK) return(err);
							if((err = cw_convert(&asound, &csound, bits)) != CWOK) return(err);
							playmode = cwstudio_play(&csound);
							if(playmode == CWPLAYING) strcpy(statustext, "Playback started.");
						}
					}
					else if(event.bstate & BUTTON2_PRESSED) {
						param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
						}
					else if(event.bstate & BUTTON3_PRESSED) {
						playmode = cwstudio_pause();
						if(playmode == CWPLAYING)
							strcpy(statustext, "Playback resumed.");
						else
							strcpy(statustext, "Playback paused.");
						}
				}
				break;
#endif

			case KEY_F(1):
			case '1':
				cwstudio_help();
				break;

			case KEY_F(2):
			case '2':
				if(csound.length) {
					i = (int) time(NULL);
					sprintf(filename, "%x.wav", i);
					if((err = cw_wavout(filename, &csound)) != CWOK) return(i);
					sprintf(filename, "%x.txt", i);
					f = fopen(filename,"w");
					fputs(text,f);
					fclose(f);
					sprintf(statustext, "Saved to %x.wav.", i);
				}
				break;

			case KEY_F(3):
			case '3':
				i = param.seed;
				cw_initparam(&param);
				param.seed = i;
				break;

			case KEY_F(4):
			case '4':
			case ' ':
				param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
				break;

			case KEY_F(5):
			case '5':
				if(playmode == CWSTOPPED) {
					cw_freesample(&asound);
					cw_freesample(&csound);
					cw_initsample(&asound, NULL);
					asound.samplerate = samplerate;
					cw_initsample(&csound, &asound);
					if((err = cw_signals(&asound, param, morsetext)) != CWOK) return(err);
					if((err = cw_convert(&asound, &csound, bits)) != CWOK) return(err);
					playmode = cwstudio_play(&csound);
					if(playmode == CWPLAYING) strcpy(statustext, "Playback started.");
				}
				break;

			case KEY_F(6):
			case '6':
				playmode = cwstudio_stop();
				strcpy(statustext, "Playback stopped.");
				break;

			case KEY_F(7):
			case '7':
				playmode = cwstudio_pause();
				if(playmode == CWPLAYING)
					strcpy(statustext, "Playback resumed.");
				else
					strcpy(statustext, "Playback paused.");
				break;

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
				break;

			case KEY_F(9):
			case '9':
				param.freq = param.freq - 100;
				RANGE(freq, 100, 4000);
				break;

			case KEY_F(10):
			case '0':
				param.freq = param.freq + 100;
				RANGE(freq, 100, 4000);
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
				break;

			case KEY_F(12):
			case '=':
				mode++;
				if(mode >= 3) mode = 0;
				break;

			case KEY_BACKSPACE:
				if(param.shape >= 20)
					param.shape = -20;
				else
					param.shape = param.shape + 5;
				break;

			case KEY_PPAGE:
				param.tempo = param.tempo + 5;
				RANGE(tempo, 5, 500);
				break;

			case KEY_NPAGE:
				param.tempo = param.tempo - 5;
				RANGE(tempo, 5, 500);
				break;

			case KEY_HOME:
				strncpy(charset, charset_backup, 256);
				chars--;
				BOUND(chars, 2, strlen(charset_backup));
				charset[chars] = '\0';
				break;

			case KEY_END:
				strncpy(charset, charset_backup, 256);
				chars++;
				BOUND(chars, 2, strlen(charset_backup));
				charset[chars] = '\0';
				break;

			case KEY_SHOME:
				if(param.dashlen >= 800)
					param.dashlen = 200;
				else
					param.dashlen = param.dashlen + 50;
				break;

			case KEY_SEND:
				if(param.spacelen >= 200)
					param.spacelen = 50;
				else
					param.spacelen = param.spacelen + 25;
				break;

			case KEY_IC:
				param.signals++;
				RANGE(signals, 1, 5);
				break;

			case KEY_DC:
				param.signals--;
				RANGE(signals, 1, 5);
				break;

			case KEY_RIGHT:
				param.cspaces++;
				RANGE(cspaces, 0, 100);
				break;

			case KEY_LEFT:
				param.cspaces--;
				RANGE(cspaces, 0, 100);
				break;

			case KEY_SRIGHT:
				param.wspaces++;
				RANGE(wspaces, 0, 100);
				break;

			case KEY_SLEFT:
				param.wspaces--;
				RANGE(wspaces, 0, 100);
				break;

			case KEY_UP:
				param.number = param.number - 5;
				RANGE(number, 5, 100);
				break;

			case KEY_DOWN:
				param.number = param.number + 5;
				RANGE(number, 5, 100);
				break;

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
				break;

			case '?':
				if(bits == 16)
					bits = 8;
				else
					bits = 16;
				break;

			case 'A':
			case 'a':
				if(param.agc >= 100)
					param.agc = 0;
				else
					param.agc = param.agc + 25;
				break;

			case 'C':
			case 'c':
				if(param.click >= 10)
					param.click = 1;
				else
					param.click = param.click + 2;
				break;

			case 'E':
			case 'e':
				if(param.even >= 10)
					param.even = 0;
				else
					param.even++;
				break;

			case 'H':
			case 'h':
				if(param.hum >= 100)
					param.hum = 0;
				else
					param.hum = param.hum + 25;
				break;

			case 'O':
			case 'o':
				if(param.odd >= 10)
					param.odd = 0;
				else
					param.odd++;
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
				break;

			case 'Q':
			case 'q':
				if(param.hand >= 100)
					param.hand = 0;
				else
					param.hand = param.hand + 20;
				break;
			}

			/*$2- Regenerate text and refresh screen after each keypress ---------------------------------------------*/

			cwstudio_regeneratetext();
			cwstudio_repaintwindows();
		}

		/* Free memory */
		endwin();
		cw_freesample(&asound);
		cw_freesample(&csound);

		/* Exit */
		return(CWOK);
	}
	else
	{
#endif

		/* If stdin is injected via pipe, read it. Otherwise, generate random group */
		if(isatty(STDIN_FILENO)) {
			if((text = cwstudio_generate_text()) == NULL) return(CWALLOC);
		}
		else {
			if((text = malloc(2048 * sizeof(char))) == NULL) return(CWALLOC);
			(void) fgets(text, 2048, stdin);
		}

		/* Encode text */
		if((morsetext = cw_encode(text)) == NULL) return(CWALLOC);

		/* output */
		if(output) {
			fprintf(stderr, "\n----------------------------------------------------\n");
			fprintf
			(
				stderr,
				"CWStudio %s (%s%s%s)\nCopyright 2009-2013 Lukasz Komsta, SP8QED\n",
				VERSION,
				CANONICAL_HOST,
				SOUND_INTERFACE,
				THREAD_INTERFACE
			);
			fprintf(stderr, "----------------------------------------------------\n");
			fprintf(stderr, "* Working at %i Hz, %i bits\n", samplerate, bits);

			if(isatty(STDIN_FILENO)) {
				switch(mode)
				{
				case 0: fprintf(stderr, "* Charset: %s\n\n", charset); break;
				case 1: fprintf(stderr, "* %i words from %i most common\n\n", param.number, wordset); break;
				case 2: fprintf(stderr, "* %i calls\n\n", param.number); break;
				}
			}
			else {
				fprintf(stderr, "* Getting text from stdin\n\n");
			}

			fprintf(stderr, "* Frequency %i Hz * Window %i samples\n", param.freq, param.window);
			if(param.even) fprintf(stderr, "* %i%% even harmonics ", param.even);
			if(param.odd) fprintf(stderr, "* %i%% odd harmonics ", param.odd);
			if(param.even || param.odd) fprintf(stderr, "\n");
			if(param.click) fprintf(stderr, "* %i dB Click ", param.click);
			if(param.hand) fprintf(stderr, "* Hand simulation %i%% ", param.hand);
			if(param.hum) fprintf(stderr, "* %i%% Hum ", param.hum);
			fprintf(stderr, "\n");
			if(param.sweepness) fprintf(stderr, "* Sweep from %i Hz, sweepness %i\n", param.sweep, param.sweepness);
			if(param.detune) fprintf(stderr, "* Detune %i%% ", param.detune);
			if(param.qsb) fprintf(stderr, "* QSB %i%% ", param.qsb);
			fprintf(stderr, "\n");
			fprintf(stderr, "* Tempo is %i wpm ", param.tempo);
			if(param.cspaces) fprintf(stderr, "* Char spacing +%i ", param.cspaces);
			if(param.wspaces) fprintf(stderr, "* Word spacing +%i ", param.wspaces);
			fprintf(stderr, "\n");
			fprintf(stderr, "* Random seed: %i ", param.seed);
			if(param.shape)
				fprintf(stderr, "* Random shape: %i \n", param.shape);
			else
				fprintf(stderr, "\n");
			if(param.signals > 1) fprintf(stderr, "* Mixing %i signals ", param.signals);
			fprintf(stderr, "\n");
			if(param.noise) {
				fprintf(stderr, "* Adding %i%% noise, %i - %i Hz ", param.noise, param.lowcut, param.highcut);
				if(param.agc) fprintf(stderr, "* %i%% AGC ", param.agc);
				fprintf(stderr, "\n");
			}

			if(param.dashlen != 300) fprintf(stderr, "* Dash length: %i%% ", param.dashlen);
			if(param.spacelen != 100) fprintf(stderr, "* Space length: %i%% ", param.spacelen);
			if((param.dashlen != 300) || (param.spacelen != 100)) fprintf(stderr, "\n");
			fprintf(stderr, "----------------------------------------------------\n\n");
		}

		fprintf(stderr, "\n%s\n\n", text);

		/*
		 * "asound" is floating sample created by library, converted sample goest to
		 * "csound"
		 */
		cw_initsample(&asound, NULL);
		asound.samplerate = samplerate;
		cw_initsample(&csound, &asound);

		/* Sound generation */
		if((err = cw_signals(&asound, param, morsetext)) != CWOK) return(err);
		if((err = cw_convert(&asound, &csound, bits)) != CWOK) return(err);

		/* If stdout is redirected somewhere, feed generated WAV file there. */
		if(isatty(STDOUT_FILENO)) {
			i = (int) time(NULL);
			sprintf(filename, "%x.wav", i);
			if((err = cw_wavout(filename, &csound)) != CWOK) return(i);
			sprintf(filename, "%x.txt", i);
			f = fopen(filename,"w");
			fputs(text,f);
			fclose(f);
		}
		else {
			fprintf(stderr, "* Redirecting sound to stdout\n\n");
			if((i = cw_wavout(NULL, &csound)) != CWOK) return(i);
		}

		fflush(stderr);

#if defined(HAVE_OSS) || defined(HAVE_PULSEAUDIO) || defined(HAVE_LIBWINMM)
		/* Play if needed */
		if(play) {
			printf("<5> - PLAY, <6> - STOP, <7> - PAUSE, Ctrl-C - EXIT\n");
			while((ch = getch()) != 3) {
				switch(ch)
				{
				case 53:	cwstudio_play(&csound); break;
				case 54:	cwstudio_stop(); break;
				case 55:	cwstudio_pause(); break;
				}
			}
		}
#endif

		/* Free memory */
		cw_freesample(&asound);
		cw_freesample(&csound);
		cw_free(text);
		cw_free(morsetext);

		/* Exit */
		return(CWOK);

#ifdef HAVE_CURSES
	}
#endif
}
