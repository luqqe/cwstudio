/*$T src/fft.c GC 1.140 10/28/11 20:45:57 */

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

/*
 =======================================================================================================================
    Fast Fourier Transform (FFT) step routine. Implemented almost from scratch, using recursive "Butterfly" algorithm.
    Needs one additional vector of temporary memory. A good compromise between code complexity and efficiency.
 =======================================================================================================================
 */
void cw_fftstep
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
)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long int	k, k1, k2, k3, k4, nh;
	floating	c, s, t0, t1;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	nh = n / 2;
	if(n > 2) {
		{
			/* Longer than 2 - perform two recursive steps and combine */
			cw_fftstep(tr, ti, tmpr, tmpi, fr, fi, nh, off, d + d);
			cw_fftstep(tr, ti, tmpr, tmpi, fr, fi, nh, off + d, d + d);
			for(k = 0; k < nh; k++) {
				{
					k1 = d * k + off;
					k2 = d * nh + k1;
					k3 = 2 * k * d + off;
					k4 = k3 + d;
					c = cw_cos(6.283185 * (floating) k / (floating) n);
					s = cw_sin(6.283185 * (floating) k / (floating) n);
					t0 = c * tmpr[k4] + s * tmpi[k4];
					t1 = c * tmpi[k4] - s * tmpr[k4];
					fr[k2] = tmpr[k3] - t0;
					fi[k2] = tmpi[k3] - t1;
					fr[k1] = tmpr[k3] + t0;
					fi[k1] = tmpi[k3] + t1;
				}
			}
		}
	}
	else {

		/* Only 2 elements, final step of recursion */
		k1 = off;
		k2 = k1 + d;
		fr[k2] = tr[k1] - tr[k2];
		fi[k2] = ti[k1] - ti[k2];
		fr[k1] = tr[k1] + tr[k2];
		fi[k1] = ti[k1] + ti[k2];
	}
}

/*
 =======================================================================================================================
    Main FFT Routine, allocating temporary memory and making top fft_step() call.
 =======================================================================================================================
 */
void cw_fft(floating *tr, floating *ti, floating *fr, floating *fi, long int n)
{
	/*~~~~~~~~~~~~~~*/
	floating	*temp;
	long int	i;
	/*~~~~~~~~~~~~~~*/

	temp = cw_malloc(2 * n * sizeof(floating));
	for(i = 0; i < 2 * n; i++) temp[i] = 0;
	cw_fftstep(tr, ti, fr, fi, temp, temp + n, n, 0, 1);
	cw_free(temp);
}

/*
 =======================================================================================================================
    Inverse FFT Routine, performing FFT again and scaling resulting coefficients.
 =======================================================================================================================
 */
void cw_ifft(floating *tr, floating *ti, floating *fr, floating *fi, long int n)
{
	/*~~~~~~~~~~~~~~~*/
	long int	nh, i;
	floating	t0, t1;
	/*~~~~~~~~~~~~~~~*/

	nh = n / 2;
	cw_fft(fr, fi, tr, ti, n);
	tr[0] /= n;
	ti[0] /= n;
	tr[nh] /= n;
	ti[nh] /= n;
	for(i = 1; i < nh; i++) {
		t0 = tr[i] / n;
		t1 = ti[i] / n;
		tr[i] = tr[n - i] / n;
		ti[i] = ti[n - i] / n;
		tr[n - i] = t0;
		ti[n - i] = t1;
	}
}
