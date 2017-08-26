#include "shine.h"

static bool entry_test(Animation *animation, Controller *con) {
    return CONTROLLER_B(*con) && con->joy_y < 100;
}

static void display(Animation *animation) {
    uint8_t intensity = pulse(animation->routine_frame, 30);
    showColor(0, 0, intensity, 8);
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

Routine shine_routine = {
    true,
    &entry_test,
    &display,
    &end_test
};
