/*$T /play.c GC 1.150 2016-12-26 17:33:58 */

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
#include "cwstudio.h"

#ifdef __DJGPP__
#include <go32.h>
#include <dpmi.h>
#include <pc.h>
#define BLOCKLEN	32768
#define SUBBLOCKLEN 8192
_go32_dpmi_seginfo			irq_backup, irq;
_go32_dpmi_seginfo			dos_buffer;
volatile int				dos_offset;

unsigned int				sb_base;
int							sb_dma;
int							sb_irq;

volatile long unsigned int	playcounter;
volatile int				dos_counter;
unsigned char				*buffer;
long unsigned int			length;
#endif
#if HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_PULSEAUDIO
#include <pulse/simple.h>
#include <pulse/error.h>
#endif
#ifdef HAVE_COREAUDIO
#include "AudioToolbox/AudioToolbox.h"
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
#ifdef HAVE_AUDIOIO
#include <sys/audioio.h>
#endif
#ifdef HAVE_SNDIO
#include <sndio.h>
#endif

volatile int					status = CWSTOPPED;
long int					counter;
char						*place;

#ifdef HAVE_PTHREAD
#include <pthread.h>
pthread_t					cwstudio_thread;
pthread_attr_t				cwstudio_attr;
#endif
#ifdef HAVE_LIBWINMM
HWAVEOUT					h;
WAVEFORMATEX				wf;
WAVEHDR						wh;
HANDLE						d;
#elif HAVE_PULSEAUDIO
static pa_sample_spec		pas;
pa_simple					*pa = NULL;
int						e;
#elif defined HAVE_SNDIO
struct sio_hdl					*h;
struct sio_par					p;
#elif defined HAVE_AUDIOIO
int				audio;
struct audio_info			info;
#elif defined HAVE_OSS
int							audio;
int							format, stereo;
int							speed;
#elif defined HAVE_COREAUDIO
#define BUFSIZE 10240
volatile static long int	offset = 0, offsetmax = 0;
AudioQueueRef				queue;

/*$3- Callback function for coreaudio - copy another part of buffer ==================================================*/

void cwstudio_callback(void *data, AudioQueueRef queue, AudioQueueBufferRef buf_ref)
{
	OSStatus			ossstatus;
	AudioQueueBuffer	*buf = buf_ref;
	int					nsamp = buf->mAudioDataByteSize;
	short				*samp = buf->mAudioData;

	if(offsetmax > offset) {
		memcpy(samp, ((char *) data) + offset, nsamp);
		offset += nsamp;
		ossstatus = AudioQueueEnqueueBuffer(queue, buf_ref, 0, NULL);
	}
	else {
		cwstudio_stop();
	}
}
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
	counter = (sample->bits / 8) * sample->length * sample->channels - 2;
	place = (char *) sample->data;

#ifdef HAVE_PULSEAUDIO
#define BUFSIZE 512
	if((sample->bits == 8))
		pas.format = PA_SAMPLE_U8;
	else
		pas.format = PA_SAMPLE_S16LE;
	pas.rate = sample->samplerate;
	pas.channels = sample->channels;
	if(!(pa = pa_simple_new(NULL, "cwgen", PA_STREAM_PLAYBACK, NULL, "playback", &pas, NULL, NULL, &e))) {
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
	pa_simple_free(pa);
#elif defined HAVE_SNDIO
	h = sio_open(NULL,SIO_PLAY,0);
	sio_initpar(&p);
	p.rate = sample->samplerate;
	p.pchan = 1;
	p.bits = sample->bits;
	sio_setpar(h,&p);
	sio_start(h);
	status = CWPLAYING;
	while((counter > 128) && (status != CWSTOPPED)) {
		while(status == CWPAUSED);
		sio_write(h,place,128);
		place = place + 128;
		counter = counter - 128;
	}
	sio_close(h);
#elif defined HAVE_AUDIOIO
	AUDIO_INITINFO(&info);
	info.mode = AUMODE_PLAY;
	info.play.encoding = AUDIO_ENCODING_SLINEAR;
	info.play.sample_rate = sample->samplerate;
	info.play.precision = sample->bits;
	info.play.channels = sample->channels;
	audio = open("/dev/sound", O_WRONLY, 0);
	ioctl(audio,AUDIO_SETINFO,&info);
	ioctl(audio,AUDIO_GETINFO,&info);
	status = CWPLAYING;
	while((counter > 0) && (status != CWSTOPPED)) {
		while(status == CWPAUSED);
		write(audio, place, 2);
		place = place + 2;
		counter = counter - 2;
	}
	close(audio);
#elif defined HAVE_OSS
	audio = open("/dev/dsp", O_WRONLY, 0);
	if(sample->bits == 8)
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

	close(audio);
#endif
	return(NULL);
}
#endif
#ifdef __DJGPP__

/* 
 =======================================================================================================================
	Parse "BLASTER" environment variable and set SB address, IRQ number and DMA Channel.
	Based on parsing function in libmikmod, by Andrew Zabolotny 
 =======================================================================================================================
*/
void cwstudio_sbinit(char *sbconfig)
{
	char	*env;
	env = getenv("BLASTER");

	sb_base = 0;
	sb_irq = 7;
	sb_dma = 1;

	while(env && *env) {
		while((*env == ' ') || (*env == '\t')) env++;
		if(!*env) break;
		switch(*env++)
		{
		case 'A':
		case 'a':
			sb_base = strtol(env, &env, 16);
			break;

		case 'I':
		case 'i':
			sb_irq = strtol(env, &env, 10);
			break;

		case 'D':
		case 'd':
			sb_dma = strtol(env, &env, 10);
			break;

		default:
			while(*env && (*env != ' ') && (*env != '\t')) env++;
			break;
		}
	}

	if(sb_base)
		sprintf(sbconfig, "/sb%03xh-irq%i-dma%i", sb_base, sb_irq, sb_dma);
	else
		strcat(sbconfig, "");
}

/* 
 =======================================================================================================================
	Allocate DOS memory of a given size, ensuring that all allocated memory lies in the same segment.
 =======================================================================================================================
*/
void cwstudio_allocate_dos(int size)
{
	_go32_dpmi_seginfo	tmp1, tmp2;
	dos_offset = 65535;
	while((dos_offset >> 16) != ((dos_offset + size - 1) >> 16)) {
		_go32_dpmi_free_dos_memory(&tmp2);
		tmp2 = tmp1;
		tmp1.size = size / 16;
		_go32_dpmi_allocate_dos_memory(&tmp1);
		dos_offset = tmp1.rm_segment << 4;
	}

	dos_buffer = tmp1;
	dos_offset = dos_buffer.rm_segment << 4;
}

/* 
 =======================================================================================================================
	Write a byte to the DSP port of Sound Blaster
 =======================================================================================================================
*/
void cwstudio_dsp_write(unsigned char Value)
{
	while((inportb(sb_base + 0xC) & 0x80) == 0x80);
	outportb(sb_base + 0xC, Value);
}

/* 
 =======================================================================================================================
	IRQ (callback) function for dos playing. Called every played SUBBLOCKLEN samples, copies another part of the
	sound to the proper part of the DOS DMA buffer.
 =======================================================================================================================
*/
void cwstudio_dosplay_irq()
{
	inportb(0x22E);
	outportb(0x20, 0x20);
	if(sb_irq == 2 || sb_irq == 10 || sb_irq == 11) outportb(0xA0, 0x20);
	playcounter += SUBBLOCKLEN;
	dos_counter++;
	dos_counter &= 3;

	if(playcounter <= (length - BLOCKLEN + 2 * SUBBLOCKLEN)) {
		dosmemput(buffer + playcounter + BLOCKLEN, SUBBLOCKLEN, dos_offset + (SUBBLOCKLEN * dos_counter));
	}
	else {
		cwstudio_stop();
	}
}

/* 
 =======================================================================================================================
	Set given DOS IRQ to a "cwstudio_dosplay_irq" function,
 =======================================================================================================================
*/
void cwstudio_irq_set(int irq_vector)
{
	irq.pm_offset = (int) cwstudio_dosplay_irq;
	irq.pm_selector = _go32_my_cs();
	_go32_dpmi_get_protected_mode_interrupt_vector(irq_vector, &irq_backup);
	_go32_dpmi_chain_protected_mode_interrupt_vector(irq_vector, &irq);
}

/* 
 =======================================================================================================================
	Reset given IRQ to the original pointer.
 =======================================================================================================================
*/
void cwstudio_irq_reset(int irq_vector)
{
	_go32_dpmi_set_protected_mode_interrupt_vector(irq_vector, &irq_backup);
}
#endif

/*
 =======================================================================================================================
    Start playback.
 =======================================================================================================================
 */
int cwstudio_play(cw_sample *sample)
{
#ifdef __DJGPP__
	unsigned int	temp_page, temp_offset;
#endif
	if(status == CWPLAYING) cwstudio_stop();
	if(status == CWSTOPPED)
	{
#ifdef __DJGPP__
		/* The DMA playing code is inspired by Steven H Don's code snippets, adapted and almost rewritten */
		playcounter = -SUBBLOCKLEN; /* counter of played bytes */
		dos_counter = -1;			/* counter of subsample segment in DMA buffer */
		cwstudio_allocate_dos(BLOCKLEN);
		buffer = sample->data;
		length = sample->length;

		/* copy initial part of the sound samples to DOS memory */
		dosmemput(buffer, BLOCKLEN, dos_offset);

		/* Set IRQ */
		if(sb_irq == 2)
			cwstudio_irq_set(0x71);
		else if(sb_irq == 10)
			cwstudio_irq_set(0x72);
		else if(sb_irq == 11)
			cwstudio_irq_set(0x73);
		else
			cwstudio_irq_set(8 + sb_irq);

		if(sb_irq == 2) outportb(0xA1, inportb(0xA1) & 253);
		if(sb_irq == 10) outportb(0xA1, inportb(0xA1) & 251);
		if(sb_irq == 11) outportb(0xA1, inportb(0xA1) & 247);
		if(sb_irq == 2 || sb_irq == 10 || sb_irq == 11)
			outportb(0x21, inportb(0x21) & 251);
		else
			outportb(0x21, inportb(0x21) &!(1 << sb_irq));

		/* Enable speaker */
		cwstudio_dsp_write(0xD1);

		/* Set samplerate */
		cwstudio_dsp_write(0x40);
		cwstudio_dsp_write(165);

		temp_page = dos_offset >> 16;
		temp_offset = dos_offset & 0xFFFF;

		outportb(0x0A, 4 | sb_dma);
		outportb(0x0C, 0);
		outportb(0x0B, 0x58 | sb_dma);
		outportb(sb_dma << 1, temp_offset & 0xFF);
		outportb(sb_dma << 1, temp_offset >> 8);

		if(sb_dma == 0) outportb(0x87, temp_page);
		if(sb_dma == 1) outportb(0x83, temp_page);
		if(sb_dma == 3) outportb(0x82, temp_page);

		/* Set BLOCKLEN length */
		outportb((sb_dma << 1) + 1, BLOCKLEN & 0xFF);
		outportb((sb_dma << 1) + 1, BLOCKLEN >> 8);

		outportb(0x0A, sb_dma);

		/* Set SUBBLOCKLEN length */
		cwstudio_dsp_write(0x48);
		cwstudio_dsp_write(SUBBLOCKLEN & 0xFF);
		cwstudio_dsp_write(SUBBLOCKLEN >> 8);

		/* Start playing */
		cwstudio_dsp_write(0x1C);
#endif
#ifdef HAVE_LIBWINMM
		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nChannels = sample->channels;
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
#elif defined HAVE_COREAUDIO
		OSStatus					ossstatus;
		AudioStreamBasicDescription fmt = { 0 };
		AudioQueueBufferRef			buf_ref;
		AudioQueueBuffer			*buf;
		fmt.mSampleRate = sample->samplerate;
		fmt.mFormatID = kAudioFormatLinearPCM;
		fmt.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
		fmt.mFramesPerPacket = 1;
		fmt.mChannelsPerFrame = sample->channels;
		fmt.mBytesPerPacket = fmt.mBytesPerFrame = 2;
		fmt.mBitsPerChannel = sample->bits;
		ossstatus = AudioQueueNewOutput
			(
				&fmt,
				cwstudio_callback,
				sample->data,
				CFRunLoopGetCurrent(),
				kCFRunLoopCommonModes,
				0,
				&queue
			);
		ossstatus = AudioQueueAllocateBuffer(queue, BUFSIZE, &buf_ref);
		buf = buf_ref;
		buf->mAudioDataByteSize = BUFSIZE;
		offsetmax = (sample->bits / 8) * sample->length - 2;
		cwstudio_callback(sample->data, queue, buf_ref);
		ossstatus = AudioQueueSetParameter(queue, kAudioQueueParam_Volume, 1.0);
		ossstatus = AudioQueueStart(queue, NULL);
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
#ifdef __DJGPP__
		cwstudio_dsp_write(0xD0);
#elif HAVE_LIBWINMM
		waveOutPause(h);
#elif HAVE_COREAUDIO
		AudioQueuePause(queue);
#endif
		status = CWPAUSED;
	}
	else if(status == CWPAUSED)
	{
#ifdef __DJGPP__
		cwstudio_dsp_write(0xD4);
#elif HAVE_LIBWINMM
		waveOutRestart(h);
#elif HAVE_COREAUDIO
		AudioQueueStart(queue, NULL);
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
#ifdef __DJGPP__
	cwstudio_dsp_write(0xD0);
	cwstudio_dsp_write(0xDA);

	if(sb_irq == 2)
		cwstudio_irq_reset(0x71);
	else if(sb_irq == 10)
		cwstudio_irq_reset(0x72);
	else if(sb_irq == 11)
		cwstudio_irq_reset(0x73);
	else
		cwstudio_irq_reset(8 + sb_irq);

	if(sb_irq == 2)
		outportb(0xA1, inportb(0xA1) | 2);
	else if(sb_irq == 10)
		outportb(0xA1, inportb(0xA1) | 4);
	else if(sb_irq == 11)
		outportb(0xA1, inportb(0xA1) | 8);
	if(sb_irq == 2 || sb_irq == 10 || sb_irq == 11)
		outportb(0x21, inportb(0x21) | 4);
	else
		outportb(0x21, inportb(0x21) | (1 << sb_irq));

	_go32_dpmi_free_dos_memory(&dos_buffer);
#endif
#ifdef HAVE_LIBWINMM
	waveOutReset(h);
	if(waveOutUnprepareHeader(h, &wh, sizeof(wh)) != MMSYSERR_NOERROR);
	if(waveOutClose(h) != MMSYSERR_NOERROR);
	CloseHandle(d);
#elif defined HAVE_COREAUDIO
	AudioQueueStop(queue, 1);
	offset = 0;
	AudioQueueReset(queue);
#elif defined HAVE_PULSE_AUDIO
	pa_simple_flush(pa, &e);
	pa_simple_free(pa);
#elif defined HAVE_OSS
	status = CWSTOPPED;

	/* Wait for thread to terminate */
	pthread_join(cwstudio_thread, NULL);
#endif
	status = CWSTOPPED;

	return(status);
}
