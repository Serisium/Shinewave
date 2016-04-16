#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "lookup.h"
#include "controller.h"
#include "libs/Neopixel.h"

typedef enum {UP, DOWN, LEFT, RIGHT, NONE} Direction;

typedef enum {IDLE, BLANK, PULSE, SIDEB, WOBBLE} State;

typedef struct {
    State state;
    Color color1;
    Color color2;
    Direction dir;
} Status;

Status *init_animation(void);

void next_frame(Status *status, Controller *controller);

#endif
