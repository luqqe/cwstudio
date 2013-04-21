#include "cwgen.h"

#define CWSTOPPED 0
#define CWPLAYING 1
#define CWPAUSED 2

extern void playsample(cw_sample *sample);
extern void *threadplay (void *arg);

extern int cwstudio_play(cw_sample *sample);
extern int cwstudio_pause();
extern int cwstudio_stop();


