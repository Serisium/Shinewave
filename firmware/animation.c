#include "animation.h"

void next_frame(Controller *controller) {
    if(CONTROLLER_B(*controller)) {
        showColor(lookup(255), 0, 0, 5);
    } else {
        showColor(lookup(255), lookup(80), lookup(150), 5);
    }
}
