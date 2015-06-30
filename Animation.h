#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <stdlib.h>
#include "libs/pt-1.4/pt.h"
#include "libs/Neopixel.h"

#include "Lookup.h"

//TODO: fix triangle wave
//#define TRI(N, PERIOD) (LOOKUP(abs(N * (NUM_SAMPLES/PERIOD) % ((NUM_SAMPLES - 1) * 2) - (NUM_SAMPLES - 1))))

#define SAW_ASC(N, PERIOD) LOOKUP((uint16_t) (N % PERIOD) * (NUM_SAMPLES - 1) / (PERIOD - 1))
#define SAW_DESC(N, PERIOD) LOOKUP((uint16_t) ((255 - N) % PERIOD) * (NUM_SAMPLES - 1) / (PERIOD - 1))

void nextFrame(uint8_t controller[]);

#include "Animation.cpp"

#endif
