#include "shine.h"

static bool entry_test(Animation *animation, Controller *con) {
    return CONTROLLER_B(*con) && con->joy_y < 100;
}

static void display(Animation *animation) {
    uint32_t time = animation->frame - animation->entry_frame;

    uint8_t intensity = pulse(time, 30);
    showColor(0, 0, intensity, 8);
}

static bool end_test(Animation *animation) {
    return animation->frame - animation->entry_frame > 30;
}

Routine shine_routine = {
    true,
    &entry_test,
    &display,
    &end_test
};
