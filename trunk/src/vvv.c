/*$T /vvv.c GC 1.150 2016-12-26 17:33:58 */

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
 This is an EXTREMELY SIMPLE example of cwgen library use in your own program.
 
 To preserve clarity, no exitcodes are checked (NOT RECOMMENDED IN REAL LIFE!)
  
 Under MinGW32 you can:
 
 gcc -v vvv.c
 gcc -o vvv.exe vvv.o cwgen.dll
 
 =======================================================================================================================
 */
int main()
{
	/*~~~~~~~~~~~~~~~~~~~~~~~*/
	char		*morsetext;
	cw_sample	asound, csound;
	cw_param	param;

	/*~~~~~~~~~~~~~~~~~~~~~~~*/
	cw_initparam(&param);
	morsetext = cw_encode("VVV = CQ DE CWSTUDIO PSE K");
	cw_initsample(&asound, NULL);
	cw_initsample(&csound, &asound);
	cw_signals(&asound, param, morsetext);
	cw_convert(&asound, &csound, 16);
	cw_wavout("vvv.wav", &csound);
	cw_freesample(&asound);
	cw_freesample(&csound);
	cw_free(morsetext);
	return(0);
}
