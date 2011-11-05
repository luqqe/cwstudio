/*$T src/cwgen.h GC 1.140 11/05/11 20:22:33 */

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
#include "config.h"
#include <stdio.h>
#ifdef HAVE_MATH_H
#include <math.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

/* Math definitions */
#define cw_sin	sinf
#define cw_pow	powf
#define cw_cos	cosf
#define cw_sqrt sqrtf
#define cw_pow	powf
#define cw_log	logf
#define cw_fabs fabsf
typedef float	floating;

/* Error codes */
#define CWOK		0
#define CWALLOC		1
#define CWFOPEN		2
#define CWFREAD		3
#define CWFWRITE	4
#define CWFCLOSE	5

/*
 -----------------------------------------------------------------------------------------------------------------------
    Sound sample types with corresponding length
 -----------------------------------------------------------------------------------------------------------------------
 */
typedef struct
{
	unsigned int	samplerate;
	unsigned int	bits;
	long int		length;
	void			*data;
} cw_sample;

/*
 -----------------------------------------------------------------------------------------------------------------------
    a set of all possible parameters of CW signal
 -----------------------------------------------------------------------------------------------------------------------
 */
typedef struct
{
	int agc;
	int click;
	int cspaces;
	int dashlen;
	int detune;
	int even;
	int freq;
	int hand;
	int highcut;
	int hum;
	int lowcut;
	int noise;
	int number;
	int odd;
	int qsb;
	int seed;
	int shape;
	int signals;
	int spacelen;
	int sweep;
	int sweepness;
	int tempo;
	int window;
	int wspaces;
} cw_param;
extern int				cw_wavout(const char *filename, cw_sample *sound);
extern void				cw_initsample(cw_sample *sample, cw_sample *ref);
extern void				cw_freesample(cw_sample *sample);
extern void				cw_initparam(cw_param *param);

/* Constants */
static const long int	NOISELEN = 131072;
extern const char		*cw_words[1000];
extern const char		*cw_calls[10][2272];

/* Random */
extern int				*cw_rand_int(int range, long int length, unsigned int seed);
extern int				*cw_rand_shaped(int range, int shape, long int length, unsigned int seed);
extern floating			*cw_rand_corr(int length, floating corr, unsigned int seed);
extern char				*cw_rand_groups(int ngroup, int shape, const char *charset, unsigned int seed);
extern char				*cw_rand_words(int nwords, int shape, int wordset, unsigned int seed);
extern char				*cw_rand_calls(int ncalls, int shape, unsigned int seed);
extern floating			*cw_rand_norm(long int length, unsigned int seed);

/* CW Encoding */
extern char				*cw_encode(const char *text);

/* Sound */
extern int				cw_signals(cw_sample *signals, cw_param param, char *text);
extern int				cw_signal(cw_sample *sound, cw_param param, char *text);
extern int				cw_convert(cw_sample *input, cw_sample *output, unsigned int bits);
extern int				cw_tone(cw_sample *atone, cw_param param, long int duration, int freq);
extern int				cw_silence(cw_sample *asilence, long int duration);
extern void				cw_append
						(
							cw_sample	*sample1,
							cw_sample	*sample2,
							long int	length,
							int			window,
							floating	amplitude
						);
extern void				cw_mix(cw_sample *sample1, cw_sample *sample2, floating amplitude);
extern int				cw_add_noise(cw_sample *sample, cw_param param);

/* Memory allocation functions */
extern void * (*cw_malloc) (size_t);
extern void (*cw_free) (void *);
extern void		cw_setalloc(void * (*newmalloc) (size_t), void (*newfree) (void *));

/* Internal functions */
extern floating *cw_rms(cw_sample *sample, int window);
extern int		cw_noisegen(cw_sample *anoise, long int duration, int low, int high);
extern void		cw_fftstep
				(
					floating	*tr,
					floating	*ti,
					floating	*fr,
					floating	*fi,
					floating	*tmpr,
					floating	*tmpi,
					long int	n,
					long int	off,
					long int	d
				);
extern void		cw_fft(floating *tr, floating *ti, floating *fr, floating *fi, long int n);
extern void		cw_ifft(floating *tr, floating *ti, floating *fr, floating *fi, long int n);
