#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <stdlib.h>
#include "controller.h"
#include "libs/Neopixel.h"

#define PULSE_LENGTH 20

#define COLOR_WHITE ((Color) {255, 255, 255})
#define COLOR_RED ((Color) {255, 0, 0})
#define COLOR_GREEN ((Color) {0, 255, 0})
#define COLOR_BLUE ((Color) {0, 0, 255})
#define COLOR_PINK ((Color) {255, 80, 150})
#define COLOR_PURPLE ((Color) {160, 32, 240})
#define COLOR_NONE ((Color) {0, 0, 0})

typedef enum {D_UP, D_DOWN, D_LEFT, D_RIGHT, D_NONE} Direction;

typedef enum {IDLE, BLANK, PULSE, BLIZZARD, SIDEB, WOBBLE} Action;

typedef struct {
    Action action;
    Color color1;
    Color color2;
    Direction dir;
    uint8_t timer;
    bool interruptable;
    uint8_t timeout;
    uint8_t pulse_length;
    uint8_t wobble_timer;
    uint8_t wobble_counter;
} State;

State *init_animation(void);

void next_frame(State *state, Controller *controller);

#endif
