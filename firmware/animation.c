#include "animation.h"

// Apply brightness to a color. Brightness is between 0 and 255
static inline Color apply_brightness(Color color, uint16_t brightness) {
    return (Color) {
        color.r * brightness / 255,
        color.g * brightness / 255,
        color.b * brightness / 255
    };
}

// Apply brightness to a color depending on the ratio of position to max
// Useful for turning a linear increase(1,2,3.../20) into a color pulse
static inline Color brightness_from_position(Color color, uint8_t position, uint8_t max) {
    return apply_brightness(color, ((max - position) % (max + 1)) * 255 / max);
}

State *init_animation(void) {
    State *init;
    init = (State*)malloc(sizeof(State));

    init->action = IDLE;
    init->color1 = (Color) {255, 255, 255};
    init->color2 = (Color) {0, 0, 0};
    init->dir = D_NONE;

    return init;
}

void next_frame(State *state, Controller *controller) {
    // Update the animation state machine
    state->timer++;

    // Test if the current animation has timed out
    if(state->timer >= state->timeout) {
        state->action = BLANK;
        state->interruptable = true;
    }

    if(state->interruptable) {
        // Check for Blizzard
        if(ANALOG_DOWN(*controller) && CONTROLLER_B(*controller)) {
            state->timer = 0;
            state->action = BLIZZARD;
            state->interruptable = false;
            state->color1 = COLOR_WHITE;
            state->color2 = COLOR_BLUE;
            state->pulse_length = 12;
            state->timeout = 90;
        } else if(CONTROLLER_Z(*controller) || CONTROLLER_A(*controller) && 
                (ANALOG_L(*controller) || ANALOG_R(*controller))) {
            state->timer = 0;
            state->action = PULSE;
            state->interruptable = true;
            state->color1 = COLOR_PURPLE;
            state->color2 = COLOR_NONE;
            state->pulse_length = 20;
            state->timeout = 20;
        }
    }


    // Push the animations to the LEDs
    if(state->action == BLIZZARD)
    {
        // Cut into two cycles: 0<->length, length+1<->length*2
        uint8_t position = state->timer % (state->pulse_length * 2);
        // First cycle, 0<->length
        if(position < state->pulse_length) {
            Color color1 = brightness_from_position(state->color1, position, state->pulse_length);
            Color color2 = brightness_from_position(state->color2, position, state->pulse_length);
            sendPixel(color1);
            sendPixel(color2);
            sendPixel(color1);
            sendPixel(color2);
            sendPixel(color1);
        } else { // Second cycle, length+1<->length*2
            // Subtract the length to fix the offset
            position = position - state->pulse_length;

            Color color1 = brightness_from_position(state->color1, position, state->pulse_length);
            Color color2 = brightness_from_position(state->color2, position, state->pulse_length);
            sendPixel(color2);
            sendPixel(color1);
            sendPixel(color2);
            sendPixel(color1);
            sendPixel(color2);
        }
    } else if(state->action == PULSE) {
        uint8_t position = state->timer % state->pulse_length;
        Color color1 = brightness_from_position(state->color1, position, state->pulse_length);
        showColor(color1);
    } else if(state->action == SIDEB) {
    } else if(state->action == WOBBLE) {
    } else if(state->action == IDLE) {
    } else if(state->action == BLANK) {
        showColor(COLOR_NONE);
    }
}
