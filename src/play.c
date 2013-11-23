/*$T src/play.c GC 1.140 04/22/13 17:13:31 */

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

#if HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_PULSEAUDIO
#include <pulse/simple.h>
#include <pulse/error.h>
#endif
#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#elif defined HAVE_SOUNDCARD_H
#include <soundcard.h>
#elif defined HAVE_MACHINE_SOUNDCARD_H
#include <machine/soundcard.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
volatile static int		status = CWSTOPPED;
unsigned long int		counter;
char					*place;

#ifdef HAVE_PTHREAD
#include <pthread.h>
pthread_t				cwstudio_thread;
pthread_attr_t			cwstudio_attr;
#endif
#ifdef HAVE_LIBWINMM
HWAVEOUT				h;
WAVEFORMATEX			wf;
WAVEHDR					wh;
HANDLE					d;
#elif HAVE_PULSEAUDIO
static pa_sample_spec	pas;
pa_simple				*pa = NULL;
int						e;
#elif defined HAVE_OSS
int						audio;
int						format, stereo;
int						speed;
#endif
#ifndef HAVE_LIBWINMM

/*
 =======================================================================================================================
    This function feeds audio data to the device in a loop, monitoring volatile "status" variable. If main thread
    changes the value, the audio is stopped or paused. This does not apply to WinMM/WaveOut, where threading is
    independent and this function is not compiled in.
 =======================================================================================================================
 */
void *cwstudio_playthread(void *arg)
{
	/*~~~~~~~~~~~~~~~~*/
	cw_sample	*sample;
	/*~~~~~~~~~~~~~~~~*/

	sample = (cw_sample *) arg;

	/*
	 * Variables used for loop feeding (place is pointer, counter is how many bytes
	 * left
	 */
	counter = (sample->bits / 8) * sample->length - 2;
	place = (char *) sample->data;

#ifdef HAVE_PULSEAUDIO
#define BUFSIZE 512
	if((sample->bits == 8))
		pas.format = PA_SAMPLE_U8;
	else
		pas.format = PA_SAMPLE_S16LE;
	pas.rate = sample->samplerate;
	pas.channels = 1;
	if(!(pa = pa_simple_new(NULL, "qrq", PA_STREAM_PLAYBACK, NULL, "playback", &pas, NULL, NULL, &e))) {
		fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(e));
	}

	while((counter > 0) && (status != CWSTOPPED)) {
		while(status == CWPAUSED);
		if(counter < BUFSIZE)
			pa_simple_write(pa, place, counter, &e);
		else
			pa_simple_write(pa, place, BUFSIZE, &e);
		place = place + BUFSIZE;
		counter = counter - BUFSIZE;
	}

	pa_simple_drain(pa, &e);
#elif defined HAVE_OSS
	audio = open("/dev/dsp", O_WRONLY, 0);
	if((sample->bits == 8))
		format = AFMT_U8;
	else
		format = AFMT_S16_LE;
	ioctl(audio, SNDCTL_DSP_SETFMT, &format);
	stereo = 0;
	ioctl(audio, SNDCTL_DSP_STEREO, &stereo);
	speed = sample->samplerate;
	ioctl(audio, SNDCTL_DSP_SPEED, &speed);
	status = CWPLAYING;
	while((counter > 0) && (status != CWSTOPPED)) {
		while(status == CWPAUSED);
		write(audio, place, 2);
		place = place + 2;
		counter = counter - 2;
	}

	if(close(audio) == -1);
#endif
	return(NULL);
}
#endif

/*
 =======================================================================================================================
    Start playback.
 =======================================================================================================================
 */
int cwstudio_play(cw_sample *sample)
{
	if(status == CWSTOPPED)
	{
#ifdef HAVE_LIBWINMM
		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nChannels = 1;
		wf.wBitsPerSample = sample->bits;
		wf.nSamplesPerSec = sample->samplerate;
		wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
		wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
		wf.cbSize = 0;
		d = CreateEvent(0, FALSE, FALSE, 0);
		if(waveOutOpen(&h, 0, &wf, (DWORD_PTR) d, 0, CALLBACK_EVENT) != MMSYSERR_NOERROR);
		wh.lpData = sample->data;
		wh.dwBufferLength = (sample->bits / 8) * sample->length - 2;
		wh.dwFlags = 0;
		wh.dwLoops = 0;
		if(waveOutPrepareHeader(h, &wh, sizeof(wh)) != MMSYSERR_NOERROR);
		ResetEvent(d);
		if(waveOutWrite(h, &wh, sizeof(wh)) != MMSYSERR_NOERROR);
#else
		/*
		 * If not WIN32, start new thread with pthread, or (if no pthread available) call
		 * function directly
		 */
#ifdef HAVE_PTHREAD
		pthread_attr_init(&cwstudio_attr);
		pthread_attr_setdetachstate(&cwstudio_attr, PTHREAD_CREATE_JOINABLE);
		pthread_create(&cwstudio_thread, NULL, &cwstudio_playthread, sample);
#else
		cwstudio_playthread(sample);
#endif
#endif
		status = CWPLAYING;
	}

	return(status);
}

/*
 =======================================================================================================================
    Pause or resume playback.
 =======================================================================================================================
 */
int cwstudio_pause()
{
	if(status == CWPLAYING)
	{
#ifdef HAVE_LIBWINMM
		waveOutPause(h);
#endif
		status = CWPAUSED;
	}
	else if(status == CWPAUSED)
	{
#ifdef HAVE_LIBWINMM
		waveOutRestart(h);
#endif
		status = CWPLAYING;
	}

	return(status);
}

/*
 =======================================================================================================================
    Stop playback
 =======================================================================================================================
 */
int cwstudio_stop()
{
#ifdef HAVE_LIBWINMM
	waveOutReset(h);
	if(waveOutUnprepareHeader(h, &wh, sizeof(wh)) != MMSYSERR_NOERROR);
	if(waveOutClose(h) != MMSYSERR_NOERROR);
	CloseHandle(d);
#elif defined HAVE_PULSE_AUDIO
	pa_simple_flush(pa, &e);
#elif defined HAVE_OSS
	status = CWSTOPPED;

	/* Wait for thread to terminate */
	pthread_join(cwstudio_thread, NULL);
#endif
	status = CWSTOPPED;

	return(status);
}
