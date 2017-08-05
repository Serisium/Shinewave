#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "../control.h"
#include "../controller.h"

// Forward definitions
typedef enum IdleStates IdleStates;
typedef enum ExitState ExitState;
typedef enum State State;
typedef struct Animation Animation;
typedef struct Routine Routine;

typedef enum IdleStates {
    GREEN_RISING, RED_FALLING,
    BLUE_RISING, GREEN_FALLING,
    RED_RISING, BLUE_FALLING
} IdleStates;

typedef enum ExitState {
    EXIT_RUNNING,
    EXIT_YIELD,
    EXIT_COMPLETE
} ExitState;

typedef enum State {
    State_Idle_Rainbow,
    State_Idle
} State;

typedef struct Animation {
    uint8_t rgb;
    uint32_t frame;
    uint32_t routine_frame;
    Routine *current_routine;
    ExitState routine_exit_state;
    IdleStates idle_state;
} Animation;

typedef struct Routine {
    bool hold_while_pressed;
    bool (*entry_test)(Animation *animation, Controller *con);
    void (*display)(Animation *animation);
    ExitState (*end_test)(Animation *animation);
} Routine;


void init_animation(Animation *animation);
void next_frame(Animation *animation, Controller *con);

#define NUM_ROUTINES    3
#include "jump.h"
#include "shine.h"
#include "idle.h"

#endif
