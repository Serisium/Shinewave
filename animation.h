#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <stdlib.h>
#include "neopixel.h"
#include "lookup.h"
#include "controller.h"
#include "control.h"

#define RESET_IDLE s->idleWait = 600		// wait 10 secs(60 fps) before going idle

typedef enum State {
    State_Idle_Rainbow,
    State_Idle
} State;

typedef struct Animation {
    uint8_t rgb;
    uint32_t frame;
    uint32_t entryFrame;
    //bool (*currentRoutine)(Animation *animation, Controller *con);
} Animation;

typedef enum routineState {
    ROUTINE_YIELD, ROUTINE_CONTINUE, ROUTINE_STOP
} RoutineState;

void init_animation(Animation *animation);
void next_frame(Animation *animation, Controller *con);

#endif
