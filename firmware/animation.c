#include "animation.h"

// Apply brightness to a color. Brightness is between 0 and 255
static Color apply_brightness(Color color, uint16_t brightness) {
    return (Color) {
        color.r * brightness / 255,
        color.g * brightness / 255,
        color.b * brightness / 255
    };
}

// Apply brightness to a color depending on the ratio of position to max
// Useful for turning a linear increase(1,2,3.../20) into a color pulse
static Color brightness_from_position(Color color, uint8_t position, uint8_t max) {
    return apply_brightness(color, ((max - position) % (max + 1)) * 255 / max);
}

static Direction get_analog_direction(Controller *controller) {
    if(ANALOG_UP(*controller)) {
        return D_UP;
    } else if(ANALOG_DOWN(*controller)) {
        return D_DOWN;
    } else if(ANALOG_LEFT(*controller)) {
        return D_LEFT;
    } else if(ANALOG_RIGHT(*controller)) {
        return D_RIGHT;
    }
    return D_NONE;
}

static Direction get_c_direction(Controller *controller) {
    if(C_UP(*controller)) {
        return D_UP;
    } else if(C_DOWN(*controller)) {
        return D_DOWN;
    } else if(C_LEFT(*controller)) {
        return D_LEFT;
    } else if(C_RIGHT(*controller)) {
        return D_RIGHT;
    }
    return D_NONE;
}

static void reset_animation(State *state) {
    state->action = IDLE;
    state->color1 = COLOR_WHITE;
    state->color2 = COLOR_NONE;
    state->dir = D_NONE;
    state->interruptable = true;
    state->idle_counter = 0;
}

static void setup_pulse(State *state) {
    state->action = PULSE;
    state->color1 = COLOR_WHITE;
    state->color2 = COLOR_NONE;
    state->dir= D_NONE;
    state->timer = 0;
    state->interruptable = true;
    state->timeout = 20;
    state->pulse_length = 20;
    state->echo = false;
}

State *init_animation(State *state) {
    reset_animation(state);
    state->brightness = 255;
    return state;
}

void next_frame(State *state, Controller *controller) {
    // Update the animation state machine
    state->timer++;

    // Test if the current animation has timed out
    if(state->timer >= state->timeout && state->action != IDLE) {
        reset_animation(state);
        state->action = BLANK;
    }

    // Test if the controller is idle
    if(state->action == BLANK && state->timer >= 0xff) {
        state->action = IDLE;
        state->timer = 0;
    }

    // Check for wobbling
    if(CONTROLLER_A(*controller) && state->wobble_timer > 0) {
        state->wobble_timer = 0;
        state->wobble_counter++;
    } else if(!CONTROLLER_A(*controller)) {
        state->wobble_timer++;
    }
    // Wobbling exit condition(in frames since last A press)
    if(state->wobble_timer >= 60) {
        state->wobble_counter = 0;
    }

    Direction analog_direction = get_analog_direction(controller);

    // If we're in Side-B, make sure B is released before repeating
    if((!state->interruptable) && (state->action == SIDEB) && (!CONTROLLER_B(*controller))) {
        state->interruptable = true;
    } // If the state can be interrupted, check all of the possible outcomes
    else if(state->interruptable) {
        Direction c_direction = get_c_direction(controller);
        // Test if brightness is being changed
        if((CONTROLLER_D_DOWN(*controller))) {
            if(state->brightness >= (255 - 32)) {
                state->brightness = 0;
            } else {
                state->brightness += 32;
            }
            setup_pulse(state);
            state->interruptable = false;
        // Check if we're wobbling
        } else if (CONTROLLER_A(*controller) && state->wobble_counter >= 7) {
            state->action = WOBBLE;
            state->color1 = COLOR_LIGHT_BLUE;
            state->color2 = COLOR_PINK;
            state->dir= D_NONE;
            state->timer = 0;
            state->interruptable = true;
            state->timeout = 20;
            state->pulse_length = 20;
            state->echo = false;
        } // Check for Blizzard(Down-B)
        else if(CONTROLLER_B(*controller) && analog_direction == D_DOWN) {
            state->action = BLIZZARD;
            state->color1 = COLOR_WHITE;
            state->color2 = COLOR_BLUE;
            state->dir= D_NONE;
            state->timer = 0;
            state->interruptable = false;
            state->timeout = 90;
            state->pulse_length = 12;
            state->echo = false;
        } // Check for jump(X or Y)
        else if(CONTROLLER_X(*controller) || CONTROLLER_Y(*controller)) {
            setup_pulse(state);
        } // Check for grab(Z, or Analog L/R and A)
        else if(CONTROLLER_Z(*controller) || (CONTROLLER_A(*controller) && 
                (ANALOG_L(*controller) || ANALOG_R(*controller)))) {
            setup_pulse(state);
            state->color1 = COLOR_PURPLE;
            state->color2 = COLOR_NONE;
        } // Check for Ice blocks(Neutral B)
        else if(CONTROLLER_B(*controller) && analog_direction == D_NONE) {
            setup_pulse(state);
            state->color1 = COLOR_LIGHT_BLUE;
            state->color2 = COLOR_NONE;
        } // Check for aerials, smashes, or tilts
        else if((CONTROLLER_A(*controller) && analog_direction != D_NONE) ||
                (c_direction != D_NONE)) {
            setup_pulse(state);
            state->color1 = COLOR_LIGHT_BLUE;
            state->color2 = COLOR_PINK;
            if(analog_direction != D_NONE) {
                state->dir= analog_direction;
            } else {
                state->dir= c_direction;
            }
            state->timeout = 40;
            state->pulse_length = 20;
            state->echo = true;
        } // Check for Side-B
        else if(CONTROLLER_B(*controller) && (analog_direction == D_LEFT || analog_direction == D_RIGHT)){
            // Check if this is the first Side-B
            if(state->action != SIDEB) {
                state->action = SIDEB;
                state->color1 = (Color) {30, 0, 0};
                state->color2 = COLOR_NONE;
                state->dir= D_DOWN;
                state->timer = 0;
                state->interruptable = false;
                state->timeout = 40;
                state->pulse_length = 10;
                state->echo = false;
            } else {
                state->timer = 0;
                state->color1.r = min(0xff, state->color1.r + 8);
            }
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
            color1 = apply_brightness(color1, state->brightness);
            Color color2 = brightness_from_position(state->color2, position, state->pulse_length);
            color2 = apply_brightness(color2, state->brightness);
            sendPixel(color1);
            sendPixel(color2);
            sendPixel(color1);
            sendPixel(color2);
            sendPixel(color1);
        } else { // Second cycle, length+1<->length*2
            // Subtract the length to fix the offset
            position = position - state->pulse_length;

            Color color1 = brightness_from_position(state->color1, position, state->pulse_length);
            color1 = apply_brightness(color1, state->brightness);
            Color color2 = brightness_from_position(state->color2, position, state->pulse_length);
            color2 = apply_brightness(color2, state->brightness);
            sendPixel(color2);
            sendPixel(color1);
            sendPixel(color2);
            sendPixel(color1);
            sendPixel(color2);
        }
    } else if(state->action == PULSE) {
        uint8_t position = state->timer;
        //uint8_t position = 0;

        Color colors[5];
        for(uint8_t i = 0; i < 5; i++) {
            if((position >= (PULSE_DELAY * i)) && (position < state->pulse_length + (PULSE_DELAY * i))) {
                colors[i] = brightness_from_position(state->color1, position - (PULSE_DELAY * i), state->pulse_length);
                colors[i] = apply_brightness(colors[i], state->brightness);
            } else if((state->echo) &&
                    (position >= (PULSE_DELAY * (i+1))) &&
                    (position < state->pulse_length + (PULSE_DELAY * (i + 1)))) {
                colors[i] = brightness_from_position(state->color2, position - (PULSE_DELAY * (i + 1)), state->pulse_length);
                colors[i] = apply_brightness(colors[i], state->brightness);
            } else {
                colors[i] = COLOR_NONE;
            }
        }

        switch(state->dir) {
            case(D_NONE):
                showColor(colors[0]);
                break;
            case(D_UP):
                sendPixel(colors[0]);
                sendPixel(colors[1]);
                sendPixel(colors[2]);
                sendPixel(colors[1]);
                sendPixel(colors[0]);
                break;
            case(D_DOWN):
                sendPixel(colors[2]);
                sendPixel(colors[1]);
                sendPixel(colors[0]);
                sendPixel(colors[1]);
                sendPixel(colors[2]);
                break;
            case(D_LEFT):
                sendPixel(colors[4]);
                sendPixel(colors[3]);
                sendPixel(colors[2]);
                sendPixel(colors[1]);
                sendPixel(colors[0]);
                break;
            case(D_RIGHT):
                sendPixel(colors[0]);
                sendPixel(colors[1]);
                sendPixel(colors[2]);
                sendPixel(colors[3]);
                sendPixel(colors[4]);
                break;
        }
    } else if(state->action == SIDEB) {
        uint8_t position = state->timer;

        Color color1;
        if((position >= PULSE_DELAY) && (position < state->pulse_length + PULSE_DELAY)) {
            color1 = brightness_from_position(state->color1, position - PULSE_DELAY , state->pulse_length);
            color1 = apply_brightness(color1, state->brightness);
        } else {
            color1 = COLOR_NONE;
        }
        showColor(color1);
    } else if(state->action == WOBBLE) {
        Color color1 = brightness_from_position(state->color1, state->timer, state->pulse_length);
        color1 = apply_brightness(color1, state->brightness);
        Color color2 = brightness_from_position(state->color2, state->timer, state->pulse_length);
        color2 = apply_brightness(color2, state->brightness);
        if(state->wobble_counter % 2 == 0) {
            sendPixel(color1);
            sendPixel(color1);
            sendPixel(COLOR_NONE);
            sendPixel(color2);
            sendPixel(color2);
        } else {
            sendPixel(color2);
            sendPixel(color2);
            sendPixel(COLOR_NONE);
            sendPixel(color1);
            sendPixel(color1);
        }
    } else if(state->action == IDLE) {
        switch(state->idle_counter) {
            case(0):
                showColor(apply_brightness((Color) {255, state->timer, 0}, state->brightness));
                break;
            case(1):
                showColor(apply_brightness((Color) {255 - state->timer, 255, 0}, state->brightness));
                break;
            case(2):
                showColor(apply_brightness((Color) {0, 255, state->timer}, state->brightness));
                break;
            case(3):
                showColor(apply_brightness((Color) {0, 255 - state->timer, 255}, state->brightness));
                break;
            case(4):
                showColor(apply_brightness((Color) {state->timer, 0, 255}, state->brightness));
                break;
            case(5):
                showColor(apply_brightness((Color) {255, 0, 255 - state->timer}, state->brightness));
                break;
            default:
                state->timer = 0;
                state->idle_counter = 0;
                break;
        }
        if(state->timer == 0xff) {
            state->idle_counter = state->idle_counter + 1;
        }
    } else if(state->action == BLANK) {
        showColor(COLOR_NONE);
    }
}
