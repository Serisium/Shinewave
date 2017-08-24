#include "animation.h"

typedef enum MB_STATE {
    MB_BLUE, MB_GREEN, MB_PURPLE, MB_ORANGE
} MB_STATE;

MB_STATE current_mb = MB_BLUE;
int debounce = 0;

void init_animation(Animation *animation)
{
    current_mb = 0;
    animation->frame = 0;
    animation->routine_frame = 0;
}

void next_frame(Animation *animation, Controller *con)
{
    if(debounce == 0 && CONTROLLER_A(*con)) {
        debounce = 15;
        current_mb = (current_mb + 1) % 4;
    } else if(debounce == 0 && CONTROLLER_B(*con)) {
        debounce = 15;
        if(current_mb == MB_BLUE) {
            current_mb = MB_ORANGE;
        } else {
            current_mb = (current_mb - 1) % 4;
        }
    } else if (debounce > 0) {
        debounce--;
    }

    switch(current_mb) {
case MB_BLUE:
        showColor(71, 109, 178, 8);
        break;
case MB_GREEN:
        showColor(62, 176, 57, 8);
        break;
case MB_PURPLE:
        showColor(114, 74, 150, 8);
        break;
case MB_ORANGE:
        showColor(245, 152, 20, 8);
        break;
default: 
        current_mb = MB_BLUE;
        showColor(71, 109, 178, 8);
        break;
    }
}
