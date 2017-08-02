#include "idle.h"

static bool entry_test(Animation *animation, Controller *con) {
    return true;
}

static void display(Animation *animation) {
    showColor(0, 0, 0, 8);
}

static bool end_test(Animation *animation) {
    return true;
}

Routine idle_routine = {
    false,
    &entry_test,
    &display,
    &end_test
};
