#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <stdlib.h>
#include "neopixel.h"
#include "lookup.h"

#define RESET_IDLE s->idleWait = 600		// wait 10 secs(60 fps) before going idle

void initAnimation(void);
void nextFrame(volatile uint8_t controller[]);

#endif
