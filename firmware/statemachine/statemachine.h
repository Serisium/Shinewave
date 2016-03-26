// The MIT License (MIT)

// Copyright (c) 2016 Reid Levenick

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <stdint.h>

#include "../controller.h"
#include "color.h"
#include "constants.h"

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
struct State_t;
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

Machine Machine_deserialize(uint8_t*);
void Machine_advance(Machine*, Controller*);
void Machine_color(Machine*, Color*);

#endif
