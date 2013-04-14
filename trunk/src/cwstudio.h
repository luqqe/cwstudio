#include "cwgen.h"

#define CWSTOPPED 0
#define CWPLAYING 1
#define CWPAUSED 2

void playsample(cw_sample *sample);
void *threadplay (void *arg);

int cwstudio_play(cw_sample *sample);
int cwstudio_pause();
int cwstudio_stop();


