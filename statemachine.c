// Copyright (c) 2015, Reid Levenick
// Use under other licenses available with permission.
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdint.h>
#include <stdlib.h>

#include "controller.h"

// Directions
typedef uint8_t Direction;
#define DIRECTION_NEUTRAL   0
#define DIRECTION_NORTH     1
#define DIRECTION_EAST      2
#define DIRECTION_SOUTH     3
#define DIRECTION_WEST      4

// Interpolation modes
typedef uint8_t Interpolation;
#define INTER_CONSTANT      0
#define INTER_BINARY        1
#define INTER_LERP          2

// Looping modes
typedef uint8_t Looping;
#define LOOP_STICK          0
#define LOOP_WRAP           1
#define LOOP_BOUNCE         2

// Exit selection modes
typedef uint8_t Selection;
#define SELECTION_ALL       0
#define SELECTION_ANY       1

// Patterns for the 5 lights
// Bits 0 through 4 bitmask which lights are on.
// 0b___00001 = far right
// 0b___00010 = second from right
// etc...
// Bits 5 through 7 bitmask which lights are reversed to travel B -> A rather than A -> B
// 0b000_____ = reverse nothing
// 0b001_____ = reverse right
// 0b010_____ = reverse center
// 0b100_____ = reverse left
typedef uint8_t Pattern;
static uint8_t Pattern_light_reversal_map[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 1, 1,
    0, 0, 1, 0, 0,
    0, 0, 1, 1, 1,
    1, 1, 0, 0, 0,
    1, 1, 0, 1, 1,
    1, 1, 1, 0, 0,
    1, 1, 1, 1, 1
};
#define PATTERN_LIGHT_ON(pattern, light) ((pattern) & (1 << light))
#define PATTERN_LIGHT_REVERSE(pattern, light) (Pattern_light_reversal_map[((pattern) >> 5) * 5 + (light)])

uint8_t Direction_inside(Direction dir, uint8_t x, uint8_t y) {
    int16_t x_ext = (int16_t)x - 127;
    int16_t y_ext = (int16_t)y - 127;
    // The stick is considered to be in neutral if it's within 5 units of the center.
    uint8_t in_neutral = (x_ext * x_ext + y_ext * y_ext) <= 25;

    switch(dir) {
        default:
        case DIRECTION_NEUTRAL:
            return in_neutral;
        case DIRECTION_NORTH:
            return !in_neutral && y_ext >= 0 && abs(x_ext) <= y_ext;
        case DIRECTION_EAST:
            return !in_neutral && x_ext >= 0 && abs(y_ext) < x_ext;
        case DIRECTION_SOUTH:
            return !in_neutral && y_ext < 0 && -abs(x_ext) >= y_ext;
        case DIRECTION_WEST:
            return !in_neutral && x_ext < 0 && -abs(y_ext) > x_ext;
    }
    //Unreachable
}

typedef struct Color_t {
    uint8_t r; uint8_t g; uint8_t b;
} Color;

void Color_emplace(Color *out, uint8_t r, uint8_t g, uint8_t b) {
    out->r = r;
    out->g = g;
    out->b = b;
}
Color Color_new(uint8_t r, uint8_t g, uint8_t b) {
    Color out;
    Color_emplace(&out, r, g, b);
    return out;
}
void Color_interpolate(Color *output, Color a, Color b, Interpolation method, uint8_t frac) {
    switch(method) {
        default:
        case INTER_CONSTANT:
            *output = a;
            break;
        case INTER_BINARY:
            if(frac >= 128) {
                *output = b;
            } else {
                *output = a;
            }
            break;
        case INTER_LERP:
            return Color_emplace(output,
                    (((uint16_t)a.r * (256 - frac)) >> 8) + (((uint16_t)b.r * frac) >> 8),
                    (((uint16_t)a.g * (256 - frac)) >> 8) + (((uint16_t)b.g * frac) >> 8),
                    (((uint16_t)a.b * (256 - frac)) >> 8) + (((uint16_t)b.b * frac) >> 8));
            break;
    }
    // Unreachable
}

// Defines an animation, including colors, fading, looping, etc
typedef struct Animation_t {
    // The color at frac = 0
    Color start;
    // The color at frac = 255
    Color finish;
    // The method with which to fade between them
    Interpolation method;
    // How much frac advances each frame
    uint8_t speed;
    // How this animation loops if at all
    Looping looping;
    // The pattern this animation displays with
    Pattern pattern;
} Animation;
typedef uint8_t p_Animation;

// Forward declare the struct so we can have a pointer to it...
typedef struct State_t State;
typedef uint8_t p_State;

// Defines what conditions need to occur to exit a state
typedef struct Exit_t {
    // What animation we move to
    p_State next;
    // How we determine whether the exit is valid (buttons are considered independently)
    Selection method;
    // If the buttons & mask != 0
    uint16_t button_mask;
    // A bitmask of which analog values are tested
    // 0b00000001 = joystick
    // 0b00000010 = c-stick
    // 0b00000100 = left min
    // 0b00001000 = left max
    // 0b00010000 = right min
    // 0b00100000 = right max
    // 0b01000000 = time
    uint8_t analog_mask;
    Direction joy_dir;
    Direction c_dir;
    uint8_t l_min;
    uint8_t l_max;
    uint8_t r_min;
    uint8_t r_max;
    uint8_t frac_limit;
} Exit;
typedef uint8_t p_Exit;

uint8_t Exit_fulfilled(Exit *exit, uint8_t anim_frac, Controller *controller) {
    uint8_t successes = 0;
    // Not sure if the processor supports popcount. On the other hand, I'd be surprised if it didn't.
    successes += __builtin_popcount(exit->button_mask & controller->button_states);

    successes += (exit->analog_mask >> 0) == 1 ? Direction_inside(exit->joy_dir, controller->joy_x, controller->joy_y) : 0;
    successes += (exit->analog_mask >> 1) == 1 ? Direction_inside(exit->c_dir, controller->c_x, controller->c_y) : 0;
    successes += (exit->analog_mask >> 2) & (controller->analog_l > exit->l_min);
    successes += (exit->analog_mask >> 3) & (controller->analog_l < exit->l_max);
    successes += (exit->analog_mask >> 4) & (controller->analog_r > exit->l_min);
    successes += (exit->analog_mask >> 5) & (controller->analog_r < exit->l_max);
    successes += (exit->analog_mask >> 6) & (anim_frac >= exit->frac_limit);

    switch(exit->method) {
        default:
        case SELECTION_ANY:
            return successes > 0;
        case SELECTION_ALL:
            return successes >= __builtin_popcount(exit->button_mask) + __builtin_popcount(exit->analog_mask);
    }
}

typedef struct Machine_t {
    // How many animations?
    uint8_t num_anims;
    // Where are they?
    Animation *anims;
    // ditto
    uint8_t num_exits;
    // ditto
    Exit *exits;
    // ditto
    uint8_t num_states;
    // ditto
    State *states;
    // This is the pointer to the array of arrays of exit IDs, which are used inside the states.
    // Kind of weird. I know. Doing this lets us make State nonreferential.
    p_Exit *exit_arrs;
    // Which state are we in?
    p_State current;
    // Where does the current state's exit array start?
    p_Exit current_exitp;
    // 0-255 progress along current animation
    uint8_t anim_frac;
    // 0, 1, -1 current loop state
    uint8_t anim_looping;
} Machine;

struct State_t {
    p_Animation anim;
    uint8_t num_exits;
};

Machine Machine_deserialize(uint8_t *data) {
    Machine machine;
    machine.num_anims = data[0];
    machine.num_exits = data[1];
    machine.num_states = data[2];

    uint16_t cursor = 3;

    machine.anims = (Animation*)&data[cursor];
    cursor += machine.num_anims * sizeof(Animation);
    machine.exits = (Exit*)&data[cursor];
    cursor += machine.num_exits * sizeof(Exit);
    machine.states = (State*)&data[cursor];
    cursor += machine.num_states * sizeof(State);
    machine.exit_arrs = (p_Exit*)&data[cursor];

    machine.current = 0;
    machine.anim_frac = 0;
    machine.anim_looping = 0;

    return machine;
}
void Machine_advance(Machine *machine, Controller *controller) {
    State *current_state = &machine->states[machine->current];
    Animation *current_anim = &machine->anims[current_state->anim];

    switch(current_anim->looping) {
        default:
        case LOOP_STICK:
            if(machine->anim_frac >= 255 - current_anim->speed) {
                machine->anim_frac = 255;
            } else {
                machine->anim_frac += current_anim->speed;
            }
            break;
        case LOOP_WRAP:
            machine->anim_frac += current_anim->speed;
            break;
        case LOOP_BOUNCE:
            if(machine->anim_looping == 0) {
                if(machine->anim_frac >= 255 - current_anim->speed) {
                    machine->anim_frac = 255;
                    machine->anim_looping = 1;
                } else {
                    machine->anim_frac += current_anim->speed;
                }
            } else /* anim_looping != 0 */ {
                if(machine->anim_frac <= 0 + current_anim->speed) {
                    machine->anim_frac = 0;
                    machine->anim_looping = 0;
                } else {
                    machine->anim_frac -= current_anim->speed;
                }
            }
            break;
    }

    // Test each exit
    uint8_t exitp = machine->current_exitp;
    for(int i = 0; i < current_state->num_exits; ++i, ++exitp) {
        Exit *current_exit = &machine->exits[machine->exit_arrs[exitp]];

        if(Exit_fulfilled(current_exit, machine->anim_frac, controller)) {
            machine->current = current_exit->next;
            machine->anim_frac = 0;
            machine->anim_looping = 0;

            machine->current_exitp = 0;
            for(int j = 0; j < machine->current; ++j) {
                machine->current_exitp += machine->states[j].num_exits;
            }

            break;
        }
    }
}
// Expects 5 colors to be available at the output location.
void Machine_color(Color *output, Machine *machine) {
    Animation *current_anim = &machine->anims[machine->states[machine->current].anim];

    for(int i = 0; i < 5; ++i) {
        if(PATTERN_LIGHT_ON(current_anim->pattern, i)) {
            uint8_t anim_frac;

            if(PATTERN_LIGHT_REVERSE(current_anim->pattern, i)) {
                anim_frac = ~machine->anim_frac;
            } else {
                anim_frac = machine->anim_frac;
            }

            Color_interpolate(output + i, current_anim->start, current_anim->finish, current_anim->method, anim_frac);
        }
    }
}
