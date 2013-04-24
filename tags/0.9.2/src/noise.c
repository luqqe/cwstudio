/*$T src/noise.c GC 1.140 11/05/11 20:37:46 */

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

/*$2- Declarations of functions in fft.c -----------------------------------------------------------------------------*/

extern void cw_fftstep
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
extern void cw_fft(floating *tr, floating *ti, floating *fr, floating *fi, long int n);
extern void cw_ifft(floating *tr, floating *ti, floating *fr, floating *fi, long int n);

/*
 =======================================================================================================================
    Generate noise with given low and high frequency cutoffs.
 =======================================================================================================================
 */
int cw_noisegen(cw_sample *anoise, long int duration, int low, int high)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	floating	*tr, *ti, *fr, *fi, max, min;
	long int	i, n2;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* Time domain real values are Gaussian normal random variates */
	if
	(
		(tr = cw_rand_norm(duration, 997)) == NULL
	||	(ti = cw_malloc(duration * sizeof(floating))) == NULL
	||	(fr = cw_malloc(duration * sizeof(floating))) == NULL
	||	(fi = cw_malloc(duration * sizeof(floating))) == NULL
	) return(CWALLOC);

	/* Time domain imaginary ones (and others) are zeroed */
	for(i = 0; i < duration; i++) {
		ti[i] = 0;
		fr[i] = 0;
		fi[i] = 0;
	}

	/* Forward FFT */
	cw_fft(tr, ti, fr, fi, duration);
	n2 = duration / 2;

	/* Get cutoff points in frequency domain */
	low = n2 * ((float) low) / (float) anoise->samplerate * 2.0;
	high = n2 * ((float) high) / (float) anoise->samplerate * 2.0;

	/* High pass filtering */
	for(i = 0; i < low; i++) {
		fr[i] = 0;
		fr[duration - 1 - i] = 0;
		fi[i] = 0;
		fi[duration - 1 - i] = 0;
	}

	/* Low pass filtering */
	for(i = 0; i < (n2 - high); i++) {
		fr[n2 + i] = 0;
		fr[n2 - i] = 0;
		fi[n2 + i] = 0;
		fi[n2 - i] = 0;
	}

	/* Go back - inverse FFT */
	cw_ifft(tr, ti, fr, fi, duration);

	/* Get maximum and minimum values of obtained stuff */
	max = -1e10;
	min = 1e10;
	for(i = 0; i < duration; i++) {
		if(tr[i] < min) min = tr[i];
		if(tr[i] > max) max = tr[i];
	}

	/* Normalize to (-1,1) range */
	for(i = 0; i < duration; i++) {
		tr[i] = (tr[i] - min) / (max - min);
		tr[i] -= 0.5;
		tr[i] += tr[i];
	}

	cw_free(ti);
	cw_free(fr);
	cw_free(fi);
	anoise->data = tr;
	return(CWOK);
}
