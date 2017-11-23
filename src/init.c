/*$T /init.c GC 1.150 2016-12-26 17:33:58 */

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
#include "cwgen.h"

/*$3- Definition of standard memory allocators =======================================================================*/

void * (*cw_malloc) (size_t) = malloc;
void (*cw_free) (void *) = free;

/*
 =======================================================================================================================
    Initialize sample structure
 =======================================================================================================================
 */
void cw_initsample(cw_sample *sample, cw_sample *ref)
{
	if(ref != NULL) {
		sample->samplerate = ref->samplerate;
		sample->bits = ref->bits;
		sample->channels = ref->channels;
	}
	else {
		sample->samplerate = 44100;
		sample->bits = 16;
		sample->channels = 1;
	}

	sample->length = 0;
	sample->data = NULL;
}

/*
 =======================================================================================================================
    Free memory used by cw_sample structure
 =======================================================================================================================
 */
void cw_freesample(cw_sample *sample)
{
	cw_free(sample->data);
	sample->data = NULL;
	sample->length = 0;
}

/*
 =======================================================================================================================
    Set own allocator for use with a library
 =======================================================================================================================
 */
void cw_setalloc(void * (*newmalloc) (size_t), void (*newfree) (void *))
{
	cw_malloc = newmalloc;
	cw_free = newfree;
}

/*
 =======================================================================================================================
    Init parameters of cw_param structure
 =======================================================================================================================
 */
void cw_initparam(cw_param *param)
{
	param->seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	param->channels = 1;
	param->window = 100;
	param->sweep = 0;
	param->sweepness = 0;
	param->click = 1;
	param->freq = 600;
	param->tempo = 80;
	param->wspaces = 0;
	param->cspaces = 0;
	param->detune = 0;
	param->hum = 0;
	param->agc = 100;
	param->noise = 100;
	param->lowcut = 300;
	param->highcut = 2400;
	param->pan = 0;
	param->qsb = 0;
	param->number = 20;
	param->signals = 3;
	param->hand = 0;
	param->even = 0;
	param->odd = 0;
	param->shape = 0;
	param->dashlen = 300;
	param->spacelen = 100;
}
