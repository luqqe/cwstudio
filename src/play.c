/*$T src/play.c GC 1.140 04/14/13 16:55:12 */


/*$6
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


#include "cwstudio.h"

#if HAVE_WINDOWS_H
#include <windows.h>
#endif
static int				status = CWSTOPPED;

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

void cwstudio_callback()
{
			if(waveOutUnprepareHeader(h, &wh, sizeof(wh)) != MMSYSERR_NOERROR);
			if(waveOutClose(h) != MMSYSERR_NOERROR);
			CloseHandle(d);
}
/*
 =======================================================================================================================
 =======================================================================================================================
 */
int cwstudio_play(cw_sample *sample)
{
#ifdef HAVE_LIBWINMM
	switch(status)
	{
	case CWPLAYING:
		{
			waveOutReset(h);
		}

	case CWSTOPPED:
		{
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

			/*
			 * if(WaitForSingleObject(d, INFINITE) != WAIT_OBJECT_0);
			 * while (playing);
			 * 
			 */
			status = CWPLAYING;
			break;
		}

	case CWPAUSED:
		{
			waveOutRestart(h);
			status = CWPLAYING;
		}
	}
#endif
	return(status);
}

/*
 =======================================================================================================================
    #elif defined HAVE_PULSEAUDIO if((sample->bits == 8)) pas.format = PA_SAMPLE_U8;
    else pas.format = PA_SAMPLE_S16LE;
    pas.rate = sample->samplerate;
    pas.channels = 1;
    if(!(pa = pa_simple_new(NULL, "qrq", PA_STREAM_PLAYBACK, NULL, "playback", &pas, NULL, NULL, &e))) {
    fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(e));
    } pa_simple_write(pa, sample->data, (sample->bits / 8) * sample->length - 2, &e);
    pa_simple_drain(pa, &e);
    #elif defined HAVE_OSS if((audio = open("/dev/dsp", O_WRONLY, 0)) == -1);
    if((sample->bits == 8)) format = AFMT_U8;
    else format = AFMT_S16_LE;
    if(ioctl(audio, SNDCTL_DSP_SETFMT, &format) == -1);
    stereo = 0;
    if(ioctl(audio, SNDCTL_DSP_STEREO, &stereo) == -1);
    speed = sample->samplerate;
    if(ioctl(audio, SNDCTL_DSP_SPEED, &speed) == -1);
    if(write(audio, sample->data, (sample->bits / 8) * sample->length - 2) == -1);
    if(close(audio) == -1);
    #endif }
 =======================================================================================================================
 */
int cwstudio_pause()
{
	if(status == CWPLAYING) {
		waveOutPause(h);
		status = CWPAUSED;
	}
	else if(status == CWPAUSED) {
		waveOutRestart(h);
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
	if((status == CWPLAYING) || (status == CWSTOPPED)) {
		waveOutReset(h);
		status = CWSTOPPED;
	}

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
	if(WaitForSingleObject(d, INFINITE) != WAIT_OBJECT_0);
	cwstudio_callback();
}
