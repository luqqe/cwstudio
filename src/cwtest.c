/*$T /cwtest.c GC 1.150 2017-12-22 21:29:50 */

/*$I0

    This file is part of CWStudio.

    Copyright 2008-2019 Lukasz Komsta, SP8QED

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
#include <time.h>
#include <stdlib.h>

/*
 =======================================================================================================================
 This is a testing procedure to catch bugs at non-common CW parameters
 =======================================================================================================================
 */
int main()
{
	/*~~~~~~~~~~~~~~~~~~~~~~~*/
	char		*morsetext, *encoded;
	cw_sample	asound, csound;
	cw_param	param;
  int samplerates[9] = { 8000, 11025, 22050, 44100, 16000, 24000, 48000, 96000, 192000 };

	/*~~~~~~~~~~~~~~~~~~~~~~~*/
	cw_initparam(&param);

	int i;

	srand(time(0));

	for(i = 0; i < 200; i++)
	{
		printf("\n\n============== TEST %04i ================\n", i);
		printf("\n\nagc %05i \n", param.agc = rand() % 2 * rand() % 101);
		printf("click %05i \n", param.click = rand() % 2 * rand() % 101);
		printf("cspaces %05i \n", param.cspaces = rand() % 2 * rand() % 10);
		printf("dashlen %05i \n", param.dashlen = 100 + rand() % 2 * rand() % 901);
		printf("detune %05i \n", param.detune = rand() % 2 * rand() % 101);
		printf("even %05i \n", param.even = rand() % 2 * rand() % 101);
		printf("freq %05i \n", param.freq = 50 + rand() % 3951);
		printf("hand %05i \n", param.hand = rand() % 2 * rand() % 101);
		printf("lowcut %05i \n", param.lowcut = 50 + rand() % 2 * rand() % 951);
		printf("highcut %05i \n", param.highcut = param.lowcut + 250 + rand() % (10000 - param.lowcut - 250));
		printf("hum %05i \n", param.hum = rand() % 2 * rand() % 101);
		printf("noise %05i \n", param.noise = rand() % 2 * rand() % 101);
		printf("odd %05i \n", param.odd = rand() % 2 * rand() % 101);
		printf("qsb %05i \n", param.qsb = rand() % 2 * rand() % 101);
		printf("channels %05i \n", param.channels = 1 + rand() % 7);
		printf("signals %05i \n", param.signals = 1 + rand() % 5);
		printf("spacelen %05i \n", param.spacelen = 20 + rand() % 2 * rand() % 281);
		printf("sweep %05i \n", param.sweep = rand() % 2 * (-4000 + rand() % 8001));
		printf("sweepness %05i \n", param.sweepness = rand() % 10000);
		printf("tempo %05i \n", param.tempo = 50 + rand() % 450);
		printf("window %05i \n", param.window = rand() % 1001);
		printf("wspaces %05i \n", param.wspaces = rand() % 2 * rand() % 10);

		morsetext = cw_rand_groups
			(
				1 + rand() % 50,
				0,
				"abstgjnokqfmzixdrhewlypvcu8219376450?!/=",
				(((unsigned int) (time(NULL) << 12)) % 32767) + 1
			);

		encoded = cw_encode(morsetext);

		cw_initsample(&asound, NULL);
		printf("samplerate %05i \n", asound.samplerate = samplerates[rand() % 9]);

		cw_initsample(&csound, &asound);

		printf("bits %05i \n", csound.bits = 8 + 8 * (rand() % 2));
		printf("\n\n%s\n", morsetext);

		cw_signals(&asound, param, encoded);
		cw_convert(&asound, &csound, csound.bits);

		//	cw_wavout("test.wav", &csound);
		cw_freesample(&asound);
		cw_freesample(&csound);
		cw_free(morsetext);
		cw_free(encoded);
	}

	return(0);
}
