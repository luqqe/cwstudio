/*$T /sound.c GC 1.150 2016-12-26 17:33:58 */

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

/*
 =======================================================================================================================
    Generate a tone sample for further inserting into final sound
 =======================================================================================================================
 */
int cw_tone(cw_sample *atone, cw_param param, long int duration, int freq)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long int	i, length;
	int			ch, j, humlength;
	floating	swp, cl, p, q, x, y, ev, od, sum;
	floating	humtable[882];
	floating	*data;

	ch = param.channels;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	if((atone->data = cw_malloc(ch * duration * sizeof(floating))) == NULL) return(CWALLOC);
	atone->length = duration;
	atone->bits = 0;
	atone->channels = param.channels;
	data = (floating *) atone->data;

	/* If frequency is not given explicitly, take from structure */
	if(freq) param.freq = freq;
	length = atone->samplerate / param.freq;

	/*
	 * If tone contains no sweep, generate only one sine period, then propagate it
	 * along time
	 */
	if(param.sweepness == 0) {
		for(i = 0; i < length; i++) for (j = 0; j < ch; j++) data[i * ch + j] = cw_sin(6.283185 * i / length);
		for(i = length; i < duration; i++) for (j = 0; j < ch; j++) data[i * ch + j] = data[(i * ch + j) % (length * ch)];
	}

	/* otherwise generate full tone with sweeping */
	else {
		swp = 1 / (floating) param.sweepness;
		p = (floating) param.sweep / (floating) param.freq;
		q = (p - 1) / swp;
		for(i = 0; i < duration; i++) {
			x = (6.283185 * (floating) i / (floating) length);
			y = cw_sin(x - q * exp(-swp * x));
			for(j = 0; j < ch; j++) data[i * ch + j] = y;
		}
	}

	/*
	 * Enhance sound with harmonics if needed. Chebyshev polynomials are used, with
	 * 1/k amplitude for k-th harmonic
	 */
	if(param.even || param.odd) {
		ev = (floating) param.even * 0.01;
		od = (floating) param.odd * 0.01;
		sum = od * 71.0 / 105.0 + ev * 11.0 / 12.0 + 1;
		for(i = 0; i < atone->length * ch; i++) {
			p = data[i];
			q = 1.0;
			x = p / sum;
			for(j = 2; j <= 7; j++) {
				swp = 2 * data[i] * p - q;
				x += swp / j * ((j % 2) ? od : ev) / sum;
				q = p;
				p = swp;
			}

			data[i] = x;
		}
	}

	/*
	 * If needed, modulate a sound with a hum, using one period of 50 Hz sine, stored
	 * in humtable
	 */
	if(param.hum) {
		humlength = (unsigned int) (atone->samplerate / 50);
		for(i = 0; i < humlength; i++) {
			humtable[i] =
				(
					(floating) param.hum *
					0.01 *
					cw_sin(6.283185307 / humlength * i) -
					(floating) param.hum *
					0.01 +
					2
				) *
				0.5;
		}

		for(i = 0; i < atone->length; i++) for(j = 0; j < ch; j++) data[i * ch + j] *= humtable[i % humlength];
	}

	/* Apply raising sine attack profile */
	for(i = 0; i <= param.window; i++) for(j = 0; j < ch; j++) data[i * ch + j] *= cw_sin((floating) i / (floating) param.window * 1.570796);

	/* If click, apply decay profile and attenuate sustain part of signal */
	if(param.click) {
		cl = 1 / cw_pow(10, (floating) param.click / 10.0);
		for(i = param.window; i < (3 * param.window); i++) {
			x = cw_cos((floating) (i - param.window) / (floating) param.window * 1.570796);
			y = 0.5 * (x + 1) * (1 - cl) + cl;
			for(j = 0; j < ch; j++) data[i * ch + j] *= y;
		}

		for(i = 3 * param.window; i < duration; i++) data[i] *= cl;
	}

	return(CWOK);
}

/*
 =======================================================================================================================
    Generate a silence sample for further inserting into final sound
 =======================================================================================================================
 */
int cw_silence(cw_sample *asilence, long int duration)
{
	/*~~~~~~~~~~~~~~*/
	long int	i;
	floating	*data;

	/*~~~~~~~~~~~~~~*/
	if((asilence->data = cw_malloc(2 * duration * sizeof(floating))) == NULL) return(CWALLOC); /* !!!!!!!!!!!!!!!!! */
	asilence->length = duration;
	asilence->bits = 0;
	asilence->channels = 2; /* !!!!!!!!!!!!!!!!!!!!!!! */
	data = (floating *) asilence->data;
	for(i = 0; i < 2 * duration; i++) data[i] = 0;
	return(CWOK);
}

/*
 =======================================================================================================================
    Append length data of sample2 to sample1, increasing length of sample1. Sample2 is sine profiled at the end with
    the given window. Appended sample is multiplied by given amplitude.
 =======================================================================================================================
 */
void cw_append(cw_sample *sample1, cw_sample *sample2, long int length, int window, floating amplitude)
{
	/*~~~~~~~~~~~~~~~~~*/
	long int	i, j;
	floating	*s1, *s2;

	/*~~~~~~~~~~~~~~~~~*/
	s1 = (floating *) sample1->data;
	s2 = (floating *) sample2->data;
	if((length == 0) || (length > sample2->length)) length = sample2->length;
	if((1 - amplitude) > 0.001)
		for(i = 0; i < length * sample1->channels; i++)
			s1[sample1->length * sample1->channels + i] = s2[i] * amplitude;
	else
		for(i = 0; i < length * sample1->channels; i++) s1[sample1->length * sample1->channels + i] = s2[i];
	sample1->length += length;
	if(window)
		for(i = 0; i < window; i++) for(j = 0; j < sample1->channels; j++)
			s1[sample1->length * sample1->channels - i - j - 1] *= cw_sin((floating) i / (floating) window * 1.570796);
}

/*
 =======================================================================================================================
    Mix sample2 with sample1 with a given amplitude. If sample2 is shorter, mix it in a loop.
 =======================================================================================================================
 */
void cw_mix(cw_sample *sample1, cw_sample *sample2, floating amplitude)
{
	/*~~~~~~~~~~~~~~~~~~*/
	long int	i, l1, l2;
	floating	a, b, c;
	floating	*s1, *s2;

	/*~~~~~~~~~~~~~~~~~~*/
	s1 = (floating *) sample1->data;
	s2 = (floating *) sample2->data;
	c = 1.0 + amplitude;
	l1 = sample1->length * sample1->channels;
	l2 = sample2->length * sample2->channels;
	for(i = 0; i < l1; i++) {
		a = s1[i];
		b = amplitude * s2[i % l2];
		s1[i] = (a + b) / c;
	}
}

/*
 =======================================================================================================================
    Add noise to a signal, with optional AGC compression simulation.
 =======================================================================================================================
 */
int cw_add_noise(cw_sample *sample, cw_param param)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long int	i,l;
	floating	a, b, c, amplitude, agc, *rmstable = NULL;
	floating	*s, *n;
	cw_sample	pinknoise;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	l = sample->length * sample->channels;
	amplitude = ((floating) param.noise) / 100.0;
	agc = ((floating) param.agc) / 100.0;
	c = 1.0 + amplitude;
	cw_initsample(&pinknoise, sample);
	if((i = cw_noisegen(&pinknoise, NOISELEN, param.lowcut / sample->channels, param.highcut / sample->channels)) != CWOK) return(i);
	s = (floating *) sample->data;
	n = (floating *) pinknoise.data;

	/* AGC requested - perform compression */
	if(param.agc) {
		if((rmstable = cw_rms(sample, 8000 * sample->channels * ((floating) sample->samplerate / 44100.0))) == NULL) return(CWALLOC);
		for(i = 0; i < l; i++) {
			a = s[i];
			b = amplitude * (1 -*(rmstable + i) * agc) * n[((i % 2) * NOISELEN) + (-(i % 2)*2+1) * (i % NOISELEN)];
			s[i] = (a + b) / (2.0 -*(rmstable + i) * (floating) agc);
		}

		cw_free(rmstable);
	}

	/* otherwise without compression */
	else {
		{
			for(i = 0; i < l; i++) {
				a = s[i];
				b = amplitude * n[i % NOISELEN];
				s[i] = (a + b) / c;
			}
		}
	}

	cw_freesample(&pinknoise);
	return(CWOK);
}

/*
 =======================================================================================================================
    Compute RMS across the sound signal using given window. Improved efficiency - each sample is processed only twice,
    regardless of window length.
 =======================================================================================================================
 */
floating *cw_rms(cw_sample *sample, int window)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	floating	*s, *result, rms, max, min;
	long int	i;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	s = (floating *) sample->data;
	if((result = cw_malloc(sample->channels * sample->length * sizeof(floating))) == NULL) return(NULL);
	rms = 0;
	max = 0;
	min = 10;
	for(i = 0; i < sample->channels * window; i++) {
		rms += (s[i] * s[i]) / window;
		*(result + i) = rms > 0 ? cw_sqrt(rms) : 0;
		if(*(result + i) > max) max = *(result + i);
		if(*(result + i) < min) min = *(result + i);
	}

	for(i = sample->channels * window; i < sample->length * sample->channels; i++) {
		rms += (s[i] * s[i]) / window;
		rms -= (s[i - sample->channels * window] * s[i - sample->channels * window]) / window;
		*(result + i) = rms > 0 ? cw_sqrt(rms) : 0;
		if(*(result + i) > max) max = *(result + i);
		if(*(result + i) < min) min = *(result + i);
	}

	for(i = 0; i < sample->length * sample->channels; i++) {
		*(result + i) -= min;
		*(result + i) /= max - min;
	}

	return(result);
}

/*
 =======================================================================================================================
    Generate a complex morse signal, consisting of several morse signals and optional noise.
 =======================================================================================================================
 */
int cw_convert(cw_sample *input, cw_sample *output, unsigned int bits)
{
	/*~~~~~~~~~~~~~~~~~~~~~*/
	unsigned long int	i, l;
	int					c;
	floating			*in;
	char				*out;

	/*~~~~~~~~~~~~~~~~~~~~~*/

	/* Allocate buffer for converted sound */
	output->bits = bits;
	if((output->data = cw_malloc(input->length * input->channels * (output->bits / 8))) == NULL) return(CWALLOC);
	output->length = input->length;
	output->channels = input->channels;
	out = (char *) output->data;
	in = (floating *) input->data;

	/* Conversion loop */
	if(output->bits == 16) {
		for(i = 0; i < input->length * input->channels - input->channels; i++) {
			c = (int) (32767 * (in[i]));
			l = 2 * i;
			out[l] = (unsigned char) c & 0xff;
			out[l + 1] = (unsigned char) (c >> 8) & 0xff;
		}
	}
	else {
		for(i = 0; i < input->length * input->channels; i++) {
			c = (int) (127 * (in[i]) + 127);
			out[i] = (unsigned char) c;
		}
	}

	return(CWOK);
}

/*
 =======================================================================================================================
    Generate one morse signal with given parameters, coding given text.
 =======================================================================================================================
 */
int cw_signal(cw_sample *sound, cw_param param, const char *text)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	cw_sample	atone, asilence;
	int			i, j, dotlen, length, freq = 0;
	long int	samples;
	floating	*detunes, *qsbs, *hands;
	floating	x, ahand, amplitude, ldash, lspace;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	detunes = NULL;
	qsbs = NULL;
	hands = NULL;
	cw_initsample(&atone, sound);
	cw_initsample(&asilence, sound);

	/* Length of the dot in samples, 12 wpm PARIS should be 100 ms */
	dotlen = 6 * sound->samplerate / param.tempo;
	length = strlen(text);

	/* Generate random detunes */
	if(param.detune)
		if((detunes = cw_rand_corr(length, 0.01, param.seed)) == NULL) return(CWALLOC);

	/* Generate random amplitudes */
	if(param.qsb)
		if((qsbs = cw_rand_corr(length, 0.01, param.seed + 1)) == NULL) return(CWALLOC);

	/* Generate random hand movements */
	if(param.hand) {
		if((hands = cw_rand_corr(length, 0.02, param.seed + 1)) == NULL) return(CWALLOC);
		for(i = 0; i < length; i++) {
			hands[i] -= 0.5;
			hands[i] *= ((floating) param.hand) / 100;
			hands[i] += 1;
		}
	}

	lspace = (floating) param.spacelen / 100.0;
	ldash = (floating) param.dashlen / 100.0;

	/* If not detuned, generate one tone used across whole signal */
	if(!param.detune)
		if((i = cw_tone(&atone, param, 2 * ldash * dotlen, 0)) != CWOK) return(i);

	/* A silence */
	if((i = cw_silence(&asilence, 3 * dotlen)) != CWOK) return(i);

	/* Compute number of samples and allocate memory for a signal */
	samples = 0;
	for(i = 0; i < length; i++) {
		ahand = param.hand ? hands[i] : 1;
		if(*(text + i) == '-')
			samples += (ldash * ahand + lspace) * dotlen;
		else if(*(text + i) == '.')
			samples += (ahand + lspace) * dotlen;
		else if((*(text + i) == ' ') || (*(text + i) == '\n'))
			samples += (2 + param.cspaces) * ahand * dotlen;
		else if(*(text + i) == '|')
			samples += (2 + param.wspaces) * (2 + param.cspaces) * ahand * dotlen;
	}

	samples += 2 * (2 + param.cspaces) * dotlen;
	if((sound->data = cw_malloc(param.channels * samples * sizeof(floating))) == NULL) return(CWALLOC);

	/* Main loop */
	for(i = 0; i < length; i++) {
		ahand = param.hand ? hands[i] : 1;

		/* Frequency of next dash/dot if signal is detuned */
		if(param.detune) {
			x = (floating) param.freq;
			x *= (*(detunes + i) - 0.5) * (floating) param.detune / 100.0 + 1;
			freq = (int) x;
		}

		/* Amplitude of next dash/dot (in dB scale) if signal has QSB */
		if(param.qsb) {
			amplitude = *(qsbs + i) * (floating) param.qsb * 0.1;
			amplitude = cw_pow(10, -amplitude);
		}
		else
			amplitude = 1;
		if(*(text + i) == '-') {
			if(param.detune) cw_tone(&atone, param, 2 * ldash * dotlen, freq);
			cw_append(sound, &atone, ldash * ahand * dotlen, param.window, amplitude);
			cw_append(sound, &asilence, lspace * dotlen, 0, 1);
			if(param.detune) cw_freesample(&atone);
		}
		else if(*(text + i) == '.') {
			if(param.detune) cw_tone(&atone, param, 2 * ldash * dotlen, freq);
			cw_append(sound, &atone, ahand * dotlen, param.window, amplitude);
			cw_append(sound, &asilence, lspace * dotlen, 0, 1);
			if(param.detune) cw_freesample(&atone);
		}
		else if((*(text + i) == ' ') || (*(text + i) == '\n')) {
			for(j = 0; j < (2 + param.cspaces); j++) cw_append(sound, &asilence, dotlen * ahand - 1, 0, 1);
		}
		else if(*(text + i) == '|') {
			for(j = 0; j < (2 + param.wspaces) * (2 + param.cspaces); j++)
				cw_append(sound, &asilence, dotlen * ahand - 1, 0, 1);
		}
	}

	/* Silence at the end */
	for(j = 0; j < 2 * (2 + param.cspaces); j++) cw_append(sound, &asilence, dotlen, 0, 1);

	/* If only one tone used, release its memory */
	if(!param.detune) cw_freesample(&atone);

	/* Freeing allocated memory */
	if(param.detune) cw_free(detunes);
	if(param.hand) cw_free(hands);
	if(param.qsb) cw_free(qsbs);

	cw_freesample(&asilence);

	return(CWOK);
}

/*
 =======================================================================================================================
    Generate a complex morse signal, consisting of several morse signals and optional noise.
 =======================================================================================================================
 */
int cw_signals(cw_sample *signals, cw_param param, const char *text)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	const floating	fmult[5] = { 1, 0.333, 1.666, 0.5, 1.5 };	/* frequency multipliers */
	const floating	tmult[5] = { 1, 1.333, 0.666, 1.25, 0.8 };	/* tempo multipliers */
	const floating	amult[5] = { 1, 0.2, 0.2, 0.5, 0.5 };		/* amplitude multipliers */
	int				i, e;
	cw_sample		anothersound;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	cw_initsample(&anothersound, signals);
	if((e = cw_signal(signals, param, text)) != CWOK) return(e);
	if(param.signals > 1) {
		for(i = 1; i < param.signals; i++) {
			param.tempo /= tmult[i - 1];
			param.tempo *= tmult[i];
			param.freq /= fmult[i - 1];
			param.freq *= fmult[i];
			anothersound.length = 0;
			if((e = cw_signal(&anothersound, param, text)) != CWOK) return(e);
			cw_mix(signals, &anothersound, amult[i]);
			cw_freesample(&anothersound);
		}
	}

	if(param.noise)
		if((e = cw_add_noise(signals, param)) != CWOK) return(e);
	return(0);
}
