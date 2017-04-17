#ifndef _LOOKUP_H
#define _LOOKUP_H

#include <avr/pgmspace.h>

#define NUM_SAMPLES 256

#define LOOKUP(N) (pgm_read_byte(&(lookup[N % NUM_SAMPLES])))

unsigned const char lookup[256] PROGMEM;

#endif
