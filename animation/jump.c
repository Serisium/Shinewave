#include "jump.h"

static bool entry_test(Animation *animation, Controller *con) {
    return CONTROLLER_X(*con) || CONTROLLER_Y(*con);
}

static void display(Animation *animation) {
    uint32_t time = animation->frame - animation->entry_frame;

    uint8_t intensity = pulse(time, 30);
    showColor(intensity, 0, 0, 8);
}

static bool end_test(Animation *animation) {
    return animation->frame - animation->entry_frame > 30;
}

Routine jump_routine = {
    true,
    &entry_test,
    &display,
    &end_test
};
