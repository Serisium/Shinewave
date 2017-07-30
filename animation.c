#include "animation.h"

bool jump(Animation *animation);
bool shine(Animation *animation);
bool idle(Animation *animation);

bool (*currentRoutine)(Animation *animation);

void init_animation(Animation *animation)
{
    animation->frame = 0;
    animation->entryFrame = 0;
    currentRoutine = &idle;
}

void next_frame(Animation *animation, Controller *con)
{
    animation->frame++;

    if(CONTROLLER_X(*con) || CONTROLLER_Y(*con)) {
        animation->entryFrame = animation->frame;
        currentRoutine = &jump;
    } else if(CONTROLLER_B(*con) && con->joy_y < 100) {
        animation->entryFrame = animation->frame;
        currentRoutine = &shine;
    }

    if((*currentRoutine)(animation)) {
        currentRoutine = &idle;
    }
}

uint8_t pulse(uint32_t position, uint32_t length) {
    return 255 * (length - position) / length;
}

bool shine(Animation *animation) {
    uint32_t time = animation->frame - animation->entryFrame;

    if(time < 30) {
        uint8_t intensity = pulse(time, 30);
        showColor(0, 0, intensity, 8);
        return false;
    } else {
        return true;
    }
}

bool jump(Animation *animation) {
    uint32_t time = animation->frame - animation->entryFrame;

    if(time < 30) {
        uint8_t intensity = pulse(time, 30);
        showColor(intensity, 0, 0, 8);
        return false;
    } else {
        return true;
    }
}

bool idle(Animation *animation) {
    showColor(0,0,0,0);
    return true;
}
