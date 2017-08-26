#include "idle.h"

static bool entry_test(Animation *animation, Controller *con) {
    if(animation->routine_exit_state == EXIT_COMPLETE) {
        return true;
    } else {
        return false;
    }
}

static void display(Animation *animation) {
    SET_BIT(PORTB, PB3);
    CLEAR_BIT(PORTB, PB3);

    uint8_t idle_amount = animation->routine_frame % 255;
    if(idle_amount == 0) {
        animation->idle_state = (animation->idle_state + 1) % 6;
    }
    if(animation->routine_frame > 300) {
        switch(animation->idle_state) {
            case GREEN_RISING:
                // green goes up
                showColor(255, idle_amount, 0, 8);
                break;
            case RED_FALLING:
                // red goes down
                showColor(255 - idle_amount, 255, 0, 8);		
                break;
            case BLUE_RISING:
                // blue goes up
                showColor(0, 255, idle_amount, 8);
                break;
            case GREEN_FALLING:
                // green goes down
                showColor(0, 255 - idle_amount, 255, 8);
                break;
            case RED_RISING:
                // red goes up
                showColor(idle_amount, 0, 255, 8);
                break;
            case BLUE_FALLING:
                // blue goes down
                showColor(255, 0, 255 - idle_amount, 8);
                break;
            default:
                animation->idle_state= 0;
                break;
        }
    } else {
        showColor(0, 0, 0, 8);
    }
}

static ExitState end_test(Animation *animation) {
    return EXIT_YIELD;
}

Routine idle_routine = {
    false,
    &entry_test,
    &display,
    &end_test
};
