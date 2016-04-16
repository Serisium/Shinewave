#include "animation.h"
#include "stdlib.h"

Status *init_animation(void) {
    Status *init;
    init = (Status*)malloc(sizeof(Status));

    init->state = IDLE;
    init->color1 = (Color) {255, 255, 255};
    init->color2 = (Color) {0, 0, 0};
    init->dir = NONE;

    return init;
}

void next_frame(Status *status, Controller *controller) {
    if(CONTROLLER_B(*controller)) {
        showColor(status->color1);
    } else {
        showColor(status->color2);
    }
}
