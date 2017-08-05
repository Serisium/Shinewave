#include "jump.h"

static bool entry_test(Animation *animation, Controller *con) {
    return CONTROLLER_X(*con) || CONTROLLER_Y(*con);
}

static void display(Animation *animation) {
    uint8_t intensity = pulse(animation->routine_frame, 30);
    showColor(intensity, 0, 0, 8);
}

static ExitState end_test(Animation *animation) {
    if(animation->routine_frame == 0) {
        return EXIT_RUNNING;
    } else if(animation->routine_frame > 30) {
        return EXIT_COMPLETE;
    } else {
        return EXIT_YIELD;
    }
}

Routine jump_routine = {
    true,
    &entry_test,
    &display,
    &end_test
};
