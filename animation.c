#include "animation.h"

void init_animation(Animation *animation)
{
}

void next_frame(Controller *con)
{
    if(CONTROLLER_X(*con)) {
        showColor(255, 0, 0, 8);
    } else {
        showColor(255, 255, 255, 8);
    }
}
