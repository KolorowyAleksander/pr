#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>     // for clock_gettime
#include <math.h>     // for fmod
#include <pvm3.h>

#define SLAVENAME "ship"
#define SLAVENUM  5
#define MAX_MSG_SIZE 255

#define MSG_INIT 777
#define MSG_TAKE 888
#define MSG_FREE 999
#define MSG_OK   101010
#define MSG_LOG  111111
