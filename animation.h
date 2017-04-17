#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <stdlib.h>
#include "neopixel.h"
#include "lookup.h"
#include "controller.h"

#define RESET_IDLE s->idleWait = 600		// wait 10 secs(60 fps) before going idle

typedef enum State {
    State_Idle_Rainbow,
    State_Idle
} State;

typedef struct Animation {
    enum State state;
    uint8_t rgb;
} Animation;

void init_animation(Animation *animation);
void next_frame(Controller *con);

#endif
