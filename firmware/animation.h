#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "controller.h"
#include "libs/Neopixel.h"

#define PULSE_LENGTH 20
#define PULSE_DELAY 6

#define COLOR_WHITE ((Color) {255, 255, 255})
#define COLOR_RED ((Color) {255, 0, 0})
#define COLOR_GREEN ((Color) {0, 255, 0})
#define COLOR_BLUE ((Color) {0, 0, 255})
#define COLOR_LIGHT_BLUE ((Color) {40, 80, 200})
#define COLOR_PINK ((Color) {255, 100, 180})
#define COLOR_PURPLE ((Color) {120, 32, 180})
#define COLOR_NONE ((Color) {0, 0, 0})

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a < _b ? _a : _b; })

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
    bool echo;
    uint8_t wobble_timer;
    uint8_t wobble_counter;
    uint8_t idle_counter;
    uint16_t brightness;
} State;

State *init_animation(State *state);

void next_frame(State *state, Controller *controller);

#endif
