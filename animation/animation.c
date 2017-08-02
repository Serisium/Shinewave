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
    animation->entry_frame = 0;
    current_routine = &idle_routine;
}

void next_frame(Animation *animation, Controller *con)
{
    animation->frame++;

    if(current_routine->hold_while_pressed && (*current_routine->entry_test)(animation, con)) {
        animation->entry_frame = animation->frame;
    }

    // Check if we're allowed to exit the current routine
    if((*current_routine->end_test)(animation)) {
        // If so, loop through our routines and enter the first valid one
        //TODO actually loop

        for(int i = 0; i < NUM_ROUTINES; i++) {
            Routine *routine = routines[i];
            if((routine->entry_test)(animation, con)) {
                animation->entry_frame = animation->frame;
                current_routine = routine;
                break;
            }
        }
    }

    (*current_routine->display)(animation);

}
