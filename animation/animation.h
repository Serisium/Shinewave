#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "../control.h"
#include "../controller.h"

// Forward definitions
typedef enum State State;
typedef struct Animation Animation;
typedef struct Routine Routine;

typedef enum State {
    State_Idle_Rainbow,
    State_Idle
} State;

typedef struct Animation {
    uint8_t rgb;
    uint32_t frame;
    uint32_t entry_frame;
    Routine *current_routine;
} Animation;

typedef struct Routine {
    bool hold_while_pressed;
    bool (*entry_test)(Animation *animation, Controller *con);
    void (*display)(Animation *animation);
    bool (*end_test)(Animation *animation);
} Routine;

void init_animation(Animation *animation);
void next_frame(Animation *animation, Controller *con);

#define NUM_ROUTINES    3
#include "jump.h"
#include "shine.h"
#include "idle.h"

#endif
