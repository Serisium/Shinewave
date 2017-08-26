#include "animation.h"

Routine *current_routine;
Routine *routines[] = {
    &jump_routine,
    &shine_routine,
    &idle_routine
};

void init_animation(Animation *animation)
{
    animation->frame = 0;
    animation->routine_frame = 0;
    current_routine = &idle_routine;
}

void next_frame(Animation *animation, Controller *con)
{
    animation->frame++;
    animation->routine_frame++;

    // Can a routine interrupt itself and hold at frame 0?
    if(current_routine->hold_while_pressed && (*current_routine->entry_test)(animation, con)) {
        animation->routine_frame = 0;
    }

    // What is the current routine's exit state?
    animation->routine_exit_state = (*current_routine->end_test)(animation);

    // Check if we're allowed to exit the current routine
    if(animation->routine_exit_state != EXIT_RUNNING) {
        // If so, loop through our routines and enter the first valid one
        for(int i = 0; i < NUM_ROUTINES; i++) {
            Routine *routine = routines[i];
            if((routine->entry_test)(animation, con) &&
                    routine != current_routine) {
                animation->routine_frame = 0;
                current_routine = routine;
                break;
            }
        }
    }

    (*current_routine->display)(animation);

}
