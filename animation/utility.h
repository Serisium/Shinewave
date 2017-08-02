#ifndef _ANIMATION_UTILITY_H
#define _ANIMATION_UTILITY_H

#include <avr/pgmspace.h>
#include "animation.h"
#include "../controller.h"
#include "neopixel.h"

#define NUM_SAMPLES 256

#define LOOKUP(N) (pgm_read_byte(&(lookup[N % NUM_SAMPLES])))

uint8_t lookup(uint8_t input);
uint8_t pulse(uint32_t position, uint32_t length);

#endif
