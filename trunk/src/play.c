/*$T src/play.c GC 1.140 04/15/13 18:32:57 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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


volatile static int				status = CWSTOPPED;
unsigned long int counter;
char *place;

#ifdef HAVE_PTHREAD
#include <pthread.h>
pthread_t cwstudio_thread;	
pthread_attr_t cwstudio_attr;
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

void *cwstudio_playthread(void *arg)
{
	cw_sample *sample;
	sample = (cw_sample*)arg;
	counter = (sample->bits / 8) * sample->length - 2 ;
	place = (char *)sample->data;

#ifdef HAVE_PULSEAUDIO	
			if((sample->bits == 8))
				pas.format = PA_SAMPLE_U8;
			else
				pas.format = PA_SAMPLE_S16LE;
			pas.rate = sample->samplerate;
			pas.channels = 1;
			if(!(pa = pa_simple_new(NULL, "qrq", PA_STREAM_PLAYBACK, NULL, "playback", &pas, NULL, NULL, &e))) {
				fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(e));
			}

			while ((counter > 0) && (status != CWSTOPPED))
			{
			while (status == CWPAUSED);
				pa_simple_write(pa, place, 2, &e);
			    pa_simple_drain(pa, &e);
				place += 2;
				counter -= 2;
			} 
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
			while ((counter > 0) && (status != CWSTOPPED))
			{
			while (status == CWPAUSED);
					write(audio, place, 2);
				place += 2;
				counter -= 2;
			} 
			if(close(audio) == -1);

#endif


			return(NULL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int cwstudio_play(cw_sample *sample)
{

	if (status == CWSTOPPED) {
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
#elif defined HAVE_OSS
#ifdef HAVE_PTHREAD
			pthread_attr_init(&cwstudio_attr);
			pthread_attr_setdetachstate(&cwstudio_attr, PTHREAD_CREATE_JOINABLE); 
			pthread_create(&cwstudio_thread, NULL, &cwstudio_playthread, sample);
			status = CWPLAYING;
#endif
#endif

			/*
			 * if(WaitForSingleObject(d, INFINITE) != WAIT_OBJECT_0);
			 * while (playing);
			 */
	}
	
	return(status);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int cwstudio_pause()
{
	if(status == CWPLAYING) {
#ifdef HAVE_LIBWINMM
		waveOutPause(h);
#endif
		status = CWPAUSED;
	}
	else if(status == CWPAUSED) {
#ifdef HAVE_LIBWINMM
		waveOutRestart(h);
#endif
		status = CWPLAYING;
	}

	return(status);
}

/*
 =======================================================================================================================
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
#endif
	status = CWSTOPPED;
	pthread_join(cwstudio_thread,NULL);

	return(status);
}

/*
 =======================================================================================================================
    Play audio sample, using WMM or OSS
 =======================================================================================================================
 */
void playsample(cw_sample *sample)
{
	cwstudio_play(sample);
#ifdef HAVE_LIBWINMM
	if(WaitForSingleObject(d, INFINITE) != WAIT_OBJECT_0);
#endif
	/* cwstudio_callback(); */
}
