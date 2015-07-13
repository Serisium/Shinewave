#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <stdlib.h>
#include "libs/pt-1.4/pt.h"
#include "libs/Neopixel.h"

#include "Lookup.h"

#define RESET_IDLE s->idleWait = 600		// wait 10 secs(60 fps) before going idle

void initAnimation(void);
void nextFrame(uint8_t controller[]);

#include "Animation.cpp"

#endif
