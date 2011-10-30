/*$T src/cwstudio.c GC 1.140 10/30/11 17:05:29 */

/*$I0

    This file is part of CWStudio.

    Copyright 2008-2011 Lukasz Komsta, SP8QED

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
#include "cwgen.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#define RANGE(x, l, u) \
	if(param.x < l) param.x = l; \
	if(param.x > u) param.x = u;
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

/*
 =======================================================================================================================
    Play audio sample, using WMM or OSS
 =======================================================================================================================
 */
void playsample(cw_sample *sample)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifdef HAVE_LIBWINMM
	HWAVEOUT		h;
	WAVEFORMATEX	wf;
	WAVEHDR			wh;
	HANDLE			d;
#elif defined HAVE_OSS
	int				audio;
	int				format, stereo;
	int				speed;
#endif
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef HAVE_LIBWINMM
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1;
	wf.wBitsPerSample = sample->bits;
	wf.nSamplesPerSec = sample->samplerate;
	wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	wf.cbSize = 0;
	d = CreateEvent(0, FALSE, FALSE, 0);
	if(waveOutOpen(&h, 0, &wf, (DWORD) d, 0, CALLBACK_EVENT) != MMSYSERR_NOERROR);
	wh.lpData = sample->data;
	wh.dwBufferLength = (sample->bits / 8) * sample->length - 2;
	wh.dwFlags = 0;
	wh.dwLoops = 0;
	if(waveOutPrepareHeader(h, &wh, sizeof(wh)) != MMSYSERR_NOERROR);
	ResetEvent(d);
	if(waveOutWrite(h, &wh, sizeof(wh)) != MMSYSERR_NOERROR);
	if(WaitForSingleObject(d, INFINITE) != WAIT_OBJECT_0);
	if(waveOutUnprepareHeader(h, &wh, sizeof(wh)) != MMSYSERR_NOERROR);
	if(waveOutClose(h) != MMSYSERR_NOERROR);
	CloseHandle(d);
#elif defined HAVE_OSS
	if((audio = open("/dev/dsp", O_WRONLY, 0)) == -1);
	if((sample->bits == 8))
		format = AFMT_U8;
	else
		format = AFMT_S16_LE;
	if(ioctl(audio, SNDCTL_DSP_SETFMT, &format) == -1);
	stereo = 0;
	if(ioctl(audio, SNDCTL_DSP_STEREO, &stereo) == -1);
	speed = sample->samplerate;
	if(ioctl(audio, SNDCTL_DSP_SPEED, &speed) == -1);
	if(write(audio, sample->data, (sample->bits / 8) * sample->length - 2) == -1);
	if(close(audio) == -1);
#endif
}

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

/*
 =======================================================================================================================
    CWStudio - command line interface
 =======================================================================================================================
 */
int main(int argc, char **argv)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	char				*text = NULL, *morsetext;
	cw_sample			asound, csound;
	cw_param			param;
	int					i, c, val, err;
	static int			play = 1, output = 1, mode = 0, wordset = 100, chars;
	static unsigned int bits = 16;
	static unsigned int samplerate = 44100;
	static char			filename[256] = "output.wav";
	static char			charset[256] = "abstgjnokqfmzixdrhewlypvcu8219376450?!/=";
	char				*envv[20], *envs, *totalv[256];
	int					envc = 0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* Initialize parameters */
	cw_initparam(&param);

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
				"a:b:B:c:C:d:E:f:g:h:H:l:n:o:O:p:P:q:r:s:S:t:v:w:W:x:X:y:",
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
			if((samplerate != 8000) && (samplerate != 11025) && (samplerate != 22050) && (samplerate != 44100))
				samplerate = 44100;
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

	if(output) {
		fprintf(stderr, "\n----------------------------------------------------\n");
		fprintf(stderr, "CWStudio %s\nCopyright 2009-2011 Lukasz Komsta, SP8QED\n", VERSION);
		fprintf(stderr, "Licensed under GPLv3\n");
		fprintf(stderr, "----------------------------------------------------\n");
		fprintf(stderr, "* Working at %i Hz, %i bits\n", samplerate, bits);
	}

	/* If stdin is injected via pipe, read it. Otherwise, generate random group */
	if(isatty(STDIN_FILENO)) {
		switch(mode)
		{
		case 0:
			if((text = cw_rand_groups(param.number, param.shape, charset, param.seed)) == NULL) return(CWALLOC);
			if(output) fprintf(stderr, "* Charset: %s\n\n", charset);
			break;

		case 1:
			if((text = cw_rand_words(param.number, param.shape, wordset, param.seed)) == NULL) return(CWALLOC);
			if(output) fprintf(stderr, "* %i words from %i most common\n\n", param.number, wordset);
			break;

		case 2:
			if((text = cw_rand_calls(param.number, param.shape, param.seed)) == NULL) return(CWALLOC);
			if(output) fprintf(stderr, "* %i calls\n\n", param.number);
			break;
		}
	}
	else {
		fprintf(stderr, "* Getting text from stdin\n\n");
		if((text = malloc(2048 * sizeof(char))) == NULL) return(CWALLOC);
		(void) fgets(text, 2048, stdin);
	}

	/* Encode text */
	if((morsetext = cw_encode(text)) == NULL) return(CWALLOC);

	/* Next part of output */
	if(output) {
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

	/* Play if needed */
	if(play) {
		fprintf(stderr, "Playing...");
		fflush(stderr);
		playsample(&csound);
		fprintf(stderr, "\n\n");
	}

	/* Free memory */
	cw_freesample(&asound);
	cw_freesample(&csound);
	cw_free(text);
	cw_free(morsetext);

	/* Exit */
	return(CWOK);
}
