/*$T src/cwstudio.h GC 1.140 04/22/13 17:13:37 */

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
#include "cwgen.h"

#define CWSTOPPED	0
#define CWPLAYING	1
#define CWPAUSED	2

extern void playsample(cw_sample *sample);
extern void *threadplay(void *arg);

extern int	cwstudio_play(cw_sample *sample);
extern int	cwstudio_pause();
extern int	cwstudio_stop();

#ifdef __DJGPP__
extern unsigned int				sb_base;
extern int							sb_dma;
extern int							sb_irq;
extern void 	cwstudio_sbinit(char *sbconfig);
#endif
