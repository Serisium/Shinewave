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
#include "statemachine.h"

#include <stdint.h>

#include "../controller.h"
#include "color.h"
#include "constants.h"

static uint8_t Exit_fulfilled(Exit *exit, uint8_t anim_frac, Controller *controller) {
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
    machine.current_exitp = 0;
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
    p_Exit exitp = machine->current_exitp;
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
void Machine_color(Machine *machine, Color *output) {
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
