#include "animation.h"

RoutineState jump(Animation *animation, Controller *con);
RoutineState idle(Animation *animation, Controller *con);

void init_animation(Animation *animation)
{
    animation->frame = 120;
    animation->lastJump = 0;
}

void next_frame(Animation *animation, Controller *con)
{
    animation->frame++;

    if(jump(animation, con) == ROUTINE_STOP) {
    } else if(idle(animation, con) == ROUTINE_STOP) {
    }
}

uint8_t pulse(uint32_t position, uint32_t length) {
    return 255 * (length - position) / length;
}

RoutineState jump(Animation *animation, Controller *con) {

    RoutineState state = ROUTINE_CONTINUE;

    if(CONTROLLER_X(*con) || CONTROLLER_Y(*con)) {
        animation->lastJump = animation->frame;
        state = ROUTINE_STOP;
    }

    if((animation->frame - animation->lastJump) < 30) {
        uint8_t intensity = pulse(animation->frame - animation->lastJump, 30);
        showColor(intensity, 0, 0, 8);
        state = ROUTINE_STOP;
    }

    return state;
}

RoutineState idle(Animation *animation, Controller *con) {
    showColor(0,0,0,0);
    return ROUTINE_STOP;
}
