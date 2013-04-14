/*$T src/cwstudio.c GC 1.140 04/14/13 18:48:24 */

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
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#elif defined HAVE_SOUNDCARD_H
#include <soundcard.h>
#elif defined HAVE_MACHINE_SOUNDCARD_H
#include <machine/soundcard.h>
#endif
#ifdef HAVE_PULSEAUDIO
#include <pulse/simple.h>
#include <pulse/error.h>
#endif
#ifdef HAVE_CURSES_H
#include <curses.h>
#elif defined HAVE_NCURSES_H
#include <ncurses.h>
#elif defined HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif
#ifdef HAVE_WINHREADS
#include <process.h>
#endif
#ifdef HAVE_LIBWINMM
#define SOUND_INTERFACE "/winmm"
#elif defined HAVE_PULSEAUDIO
#define SOUND_INTERFACE "/pulseaudio"
#elif defined HAVE_OSS
#define SOUND_INTERFACE "/oss"
#else
#define SOUND_INTERFACE ""
#endif
#ifdef HAVE_PROCESS_H
#define THREAD_INTERFACE	"/winthread"
#elif defined HAVE_PTHREAD
#define THREAD_INTERFACE	"/pthread"
#else
#define THREAD_INTERFACE	""
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
static int			playmode = CWSTOPPED;
static char			*text = NULL, *morsetext = NULL;
static cw_sample	asound, csound;
static cw_param		param;
static int			play = 1, output = 1, mode = 0, wordset = 100, chars;
static unsigned int bits = 16;
static unsigned int samplerate = 44100;
static char			filename[256] = "output.wav";
static char			charset[256] = "abstgjnokqfmzixdrhewlypvcu8219376450?!/=";
#ifdef HAVE_CURSES
static WINDOW		*win_title, *win_param, *win_text;
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

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void cwstudio_resetwindows()
{
	/*~~~~~~~~~~~*/
	int ncol, nrow;
	/*~~~~~~~~~~~*/

#define SPLIT	ncol / 2 + 8
	endwin();
	refresh();
	initscr();
	cbreak();
	noecho();
	getmaxyx(stdscr, nrow, ncol);

	if(has_colors()) {
		start_color();
		init_color(COLOR_GREEN, 0, 200, 0);
		init_pair(1, COLOR_YELLOW, COLOR_RED);
		init_pair(2, COLOR_YELLOW, COLOR_BLUE);
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
 =======================================================================================================================
 */
void cwstudio_repaintwindows()
{
	werase(win_param);

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
	if(param.hand) wprintw(win_param, "* Hand simulation %i%% ", param.detune);
	if(param.hum) wprintw(win_param, "* %i%% Hum ", param.hum);
	wprintw(win_param, "\n");
	if(param.sweepness) wprintw(win_param, "* Sweep from %i Hz, sweepness %i\n", param.sweep, param.sweepness);
	if(param.detune) wprintw(win_param, "* Detune %i%% ", param.detune);
	if(param.qsb) wprintw(win_param, "* QSB %i%% ", param.detune);
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
}

/*
 =======================================================================================================================
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
	/*~~~~~~~~~~~*/
	int ch, i, err;
	/*~~~~~~~~~~~*/

	/* Initialize parameters */
	cw_initparam(&param);

	/* Parse environmental and command line parameters */
	cwstudio_parseparam(argc, argv);

#ifdef HAVE_CURSES
	if(play) {
		signal(SIGWINCH, cwstudio_resizeterm);

		chars = 41;
		cwstudio_resetwindows();
		cwstudio_regeneratetext();
		cwstudio_repaintwindows();

		while((ch = wgetch(win_param)) != KEY_F(1)) {
			switch(ch)
			{
			case ' ':
				param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
				break;

			case KEY_HOME:
				i = param.seed;
				cw_initparam(&param);
				param.seed = i;
				break;

			case KEY_PPAGE:
				param.tempo = param.tempo + 5;
				RANGE(tempo, 5, 500);
				break;

			case KEY_NPAGE:
				param.tempo = param.tempo - 5;
				RANGE(tempo, 5, 500);
				break;

			case KEY_LEFT:
				strcpy(charset, "abstgjnokqfmzixdrhewlypvcu8219376450?!/=");
				chars--;
				BOUND(chars, 1, 41);
				charset[chars] = '\0';
				break;

			case KEY_RIGHT:
				strcpy(charset, "abstgjnokqfmzixdrhewlypvcu8219376450?!/=");
				chars++;
				BOUND(chars, 1, 41);
				charset[chars] = '\0';
				break;

			case KEY_SRIGHT:
				param.cspaces++;
				RANGE(cspaces, 0, 100);
				break;

			case KEY_SLEFT:
				param.cspaces--;
				RANGE(cspaces, 0, 100);
				break;

			case KEY_UP:
				param.number = param.number - 5;
				RANGE(number, 5, 100);
				break;

			case KEY_DOWN:
				param.number = param.number + 5;
				RANGE(number, 5, 100);
				break;

			case KEY_F(12):
				mode++;
				if(mode == 3) mode = 0;
				break;

			case 'n':
				if (param.noise == 100) param.noise = 0;
				else param.noise = param.noise + 25;
				break;

			case 'a':
				if (param.agc == 100) param.agc = 0;
				else param.agc = param.agc + 25;
				break;
				
			case 'N':
				if (param.lowcut == 300) { param.lowcut = 100; param.highcut = 6000; }
				else { param.lowcut = 300; param.highcut = 2400; }
				break;
				
			case KEY_F(5):
				if((playmode == CWPLAYING) || (playmode == CWPAUSED)) playmode = cwstudio_stop();
				cw_freesample(&asound);
				cw_freesample(&csound);
				cw_initsample(&asound, NULL);
				asound.samplerate = samplerate;
				cw_initsample(&csound, &asound);
				if((err = cw_signals(&asound, param, morsetext)) != CWOK) return(err);
				if((err = cw_convert(&asound, &csound, bits)) != CWOK) return(err);
				playmode = cwstudio_play(&csound);
				break;

			case KEY_F(6):
				playmode = cwstudio_stop();
				break;

			case KEY_F(7):
				playmode = cwstudio_pause();
				break;
			}

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
			fprintf(stderr, "Licensed under GPLv3\n");
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
			if(param.hand) fprintf(stderr, "* Hand simulation %i%% ", param.detune);
			if(param.hum) fprintf(stderr, "* %i%% Hum ", param.hum);
			fprintf(stderr, "\n");
			if(param.sweepness) fprintf(stderr, "* Sweep from %i Hz, sweepness %i\n", param.sweep, param.sweepness);
			if(param.detune) fprintf(stderr, "* Detune %i%% ", param.detune);
			if(param.qsb) fprintf(stderr, "* QSB %i%% ", param.detune);
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
			if((i = cw_wavout(filename, &csound)) != CWOK) return(i);
		}
		else {
			fprintf(stderr, "* Redirecting sound to stdout\n\n");
			if((i = cw_wavout(NULL, &csound)) != CWOK) return(i);
		}

		fflush(stderr);

#if defined(HAVE_OSS) || defined(HAVE_PULSEAUDIO) || defined(HAVE_LIBWINMM)
		/* Play if needed */
		if(play) {
			fprintf(stderr, "Playing...");
			fflush(stderr);
			playsample(&csound);
			fprintf(stderr, "\n\n");
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
