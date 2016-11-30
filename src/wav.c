/*$T src/wav.c GC 1.140 10/28/11 20:52:16 */

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
    Write a sound sample to WAV file. If filename is NULL, WAV file is written to stdout preserving the binary mode.
 =======================================================================================================================
 */
int cw_wavout(const char *filename, cw_sample *sound)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned long int	l;
	/* WAV header */
	unsigned char		header[45] =
		("\x52\x49\x46\x46\x00\x00\x00\x00\x57\x41\x56\x45\x66\x6d\x74\x20\x10\x00\x00\x00\x01\x00\x01\x00\x44\xac\x00\x00\x88\x58\x01\x00\x02\x00\x10\x00\x64\x61\x74\x61\x00\x00\x00\x00");
	FILE				*f;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	l = sound->samplerate;
	header[24] = (unsigned char) l & 0xff;
	header[25] = (unsigned char) (l >> 8) & 0xff;
	header[26] = (unsigned char) (l >> 16) & 0xff;
	header[27] = (unsigned char) (l >> 24) & 0xff;
	l = sound->bits;
	header[34] = (unsigned char) l & 0xff;
	header[35] = (unsigned char) (l >> 8) & 0xff;

	/* Store length of the sound sample in the header */
	l = (sound->bits / 8) * sound->length + 36;
	header[4] = (unsigned char) l & 0xff;
	header[5] = (unsigned char) (l >> 8) & 0xff;
	header[6] = (unsigned char) (l >> 16) & 0xff;
	header[7] = (unsigned char) (l >> 24) & 0xff;
	l = (sound->bits / 8) * sound->length;
	header[40] = (unsigned char) l & 0xff;
	header[41] = (unsigned char) (l >> 8) & 0xff;
	header[42] = (unsigned char) (l >> 16) & 0xff;
	header[43] = (unsigned char) (l >> 24) & 0xff;

	/* Bitrate (bug fixed since 0.9.5) */
	l = sound->samplerate * 1 * (sound->bits / 8);  
	header[28] = (unsigned char) l & 0xff;
	header[29] = (unsigned char) (l >> 8) & 0xff;
	header[30] = (unsigned char) (l >> 16) & 0xff;
	header[31] = (unsigned char) (l >> 24) & 0xff;


	if(filename != NULL) {
		if((f = fopen(filename, "wb+")) == NULL) {
			return(CWFOPEN);
		}
	}
	else
	{
#if _WIN32
		_setmode(_fileno(stdout), _O_BINARY);
#endif
		f = stdout;
	}

	if(fwrite(header, 1, 44, f) < 44) return(CWFWRITE);
	if(fwrite(sound->data, (sound->bits / 8), sound->length, f) < sound->length) return(CWFWRITE);
	if(filename != NULL) {
		if(fclose(f)) return(CWFCLOSE);
	}
	else {
		fflush(stdout);
#if _WIN32
		_setmode(_fileno(stdout), _O_TEXT);
#endif
	}

	return(CWOK);
}
