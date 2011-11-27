/*$T src/random.c GC 1.140 10/29/11 13:34:15 */

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
    Generate standard normal pseudorandom numbers using Box-Muller transform of uniform deviates.
 =======================================================================================================================
 */
floating *cw_rand_norm(long int length, unsigned int seed)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	int			*uniform;
	long int	i;
	floating	*normal, u1, u2, r;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	if((uniform = cw_rand_int(1000, length + length, seed)) == NULL) return(NULL);
	if((normal = cw_malloc(length * sizeof(floating))) == NULL) return(NULL);
	for(i = 0; i < length; i++) {
		u1 = ((floating) (uniform[2 * i] + 1)) / 1000.0;
		u2 = ((floating) (uniform[2 * i + 1] + 1)) / 1000.0;
		r = cw_sqrt(-2 * cw_log(u1));
		normal[i] = r * cw_cos(6.283185 * u2);
	}

	cw_free(uniform);
	return(normal);
}

/*
 =======================================================================================================================
    Generate random integers of given shape. Positive shape value increases probability of higher values, negative
    increase the lower ones.
 =======================================================================================================================
 */
int *cw_rand_shaped(int range, int shape, long int length, unsigned int seed)
{
	/*~~~~~~~~~~~~~~~~~*/
	int			*pvector;
	long int	i;
	floating	*normals;
	/*~~~~~~~~~~~~~~~~~*/

	/* If shape parameter is equal to zero, the result is equal to cw_rand_int */
	if(shape == 0) return(cw_rand_int(range, length, seed));

	/* Memory allocation */
	if((pvector = (int *) cw_malloc(length * sizeof(int))) == NULL) return(NULL);

	/* Generate equivalent length of random numbers */
	if((normals = cw_rand_norm(length, seed)) == NULL) return(NULL);

	/* Transform to integers */
	for(i = 0; i < length; i++) {
		normals[i] = cw_fabs(normals[i]) / (floating) (abs(shape) / 3.0) * (floating) range;
		pvector[i] = (((int) normals[i]) % range);
	}

	/* If negative shape, revert the result */
	if(shape > 0)
		for(i = 0; i < length; i++) pvector[i] = range - 1 - pvector[i];

	return(pvector);
}

/*
 =======================================================================================================================
    Generate random integers of given range, generated using given seed. Algorithm "Multiply-With-Carry" of G.
    Marsaglia
 =======================================================================================================================
 */
int *cw_rand_int(int range, long int length, unsigned int seed)
{
	/*~~~~~~~~~~~~~~~~~~~~~*/
	int				*pvector;
	long int		i;
	unsigned int	seed2;
	/*~~~~~~~~~~~~~~~~~~~~~*/

	if((pvector = (int *) cw_malloc(length * sizeof(int))) == NULL) return(NULL);
	seed2 = seed;
	for(i = 0; i < length; i++) {
		seed = 36969 * (seed & 65535) + (seed >> 16);
		seed2 = 18000 * (seed2 & 65535) + (seed2 >> 16);
		*(pvector + i) = ((seed << 16) + seed2) % range;
	}

	return(pvector);
}

/*
 =======================================================================================================================
    Generate autocorrelated uniform random floating variables using given seed. Used to simulate a drift (QSB, detune).
 =======================================================================================================================
 */
floating *cw_rand_corr(int length, floating corr, unsigned int seed)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	int			*runif;
	int			i;
	floating	max, min, x, u, w, *pvector;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	max = 0;
	min = 1;
	if((pvector = (floating *) cw_malloc(length * sizeof(floating))) == NULL) return(NULL);
	if((runif = cw_rand_int(1000, length, seed)) == NULL) return(NULL);
	u = 0.9999;
	for(i = 0; i < length; i++) {
		w = (floating) * (runif + i) / 1000.0 * corr;
		x = u + w;
		if(x <= corr)
			*(pvector + i) = x * x / (2 * corr);
		else if((x > corr) && (x <= 1))
			*(pvector + i) = (2 * x - corr) / 2;
		else
			*(pvector + i) = 1 - ((1 + corr - x) * (1 + corr - x)) / (2 * corr);
		u = *(pvector + i);
		if(u < min) min = u;
		if(u > max) max = u;
	}

	cw_free(runif);
	for(i = 0; i < length; i++) {
		*(pvector + i) -= min;
		*(pvector + i) /= max - min;
	}

	return(pvector);
}

/*
 =======================================================================================================================
    Generate random groups of given charset, started by VVV
 =======================================================================================================================
 */
char *cw_rand_groups(int ngroup, int shape, const char *charset, unsigned int seed)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	int		length, charsetlength;
	char	*pgroups;
	int		*randoms, i;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	length = (ngroup + 1) * 6 * sizeof(char);
	charsetlength = strlen(charset);
	if((pgroups = (char *) cw_malloc(length + 1)) == NULL) return(NULL);
	*(pgroups + length) = '\x00';
	if((randoms = cw_rand_shaped(charsetlength, shape, length, seed)) == NULL) return(NULL);
	for(i = 0; i < length; i++) strncpy(pgroups + i, &charset[*(randoms + i)], 1);
	strncpy(pgroups, "vvv =\n", 6);
	for(i = 2; i < ngroup + 2; i++) {
		if((i - 1) % 5)
			*(pgroups + (6 * i) - 1) = ' ';
		else
			*(pgroups + (6 * i) - 1) = '\n';
	}

	cw_free(randoms);
	return(pgroups);
}

/*
 =======================================================================================================================
    Generate group of random words, started by VVV
 =======================================================================================================================
 */
char *cw_rand_words(int nwords, int shape, int wordset, unsigned int seed)
{
	/*~~~~~~~~~~~~~~~~*/
	int		length;
	char	*pwords;
	int		*randoms, i;
	/*~~~~~~~~~~~~~~~~*/

	if((randoms = cw_rand_shaped(wordset, shape, nwords, seed)) == NULL) return(NULL);
	length = 6;
	for(i = 0; i < nwords; i++) length += strlen(cw_words[randoms[i]]) + 2;
	if((pwords = (char *) cw_malloc(length + 1)) == NULL) return(NULL);
	strcpy(pwords, "vvv =\n");
	for(i = 0; i < nwords; i++) {
		strcat(pwords, cw_words[*(randoms + i)]);
		if((i > 0) && ((i % 5) == 0))
			strcat(pwords, "\n");
		else
			strcat(pwords, " ");
	}

	cw_free(randoms);
	return(pwords);
}

/*
 =======================================================================================================================
    Generate group of random calls, started by VVV
 =======================================================================================================================
 */
char *cw_rand_calls(int ncalls, int shape, unsigned int seed)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	int		length;
	char	*pcalls;
	int		*randoms, i, ind1, ind2;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	if((randoms = cw_rand_shaped(22720, shape, ncalls, seed)) == NULL) return(NULL);
	length = 6;
	for(i = 0; i < ncalls; i++) {
		ind1 = (int) (randoms[i] / 22720);
		ind2 = randoms[i] % 22720;
		length += strlen(cw_calls[ind1][ind2]) + 2;
	}

	if((pcalls = (char *) cw_malloc(length + 1)) == NULL) return(NULL);
	strcpy(pcalls, "vvv =\n");
	for(i = 0; i < ncalls; i++) {
		ind1 = (int) (randoms[i] / 22720);
		ind2 = randoms[i] % 22720;
		strcat(pcalls, cw_calls[ind1][ind2]);
		if((i > 0) && ((i % 5) == 0))
			strcat(pcalls, "\n");
		else
			strcat(pcalls, " ");
	}

	cw_free(randoms);
	return(pcalls);
}
