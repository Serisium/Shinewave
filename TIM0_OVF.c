#include "TIM0_OVF.h"

volatile extern uint8_t byte_index, bitmask, is_done;

ISR(TIM0_OVF_vect) {
    disable_timer0();
    is_done = 1;

    bitmask = 0x01;
    byte_index = 0;
    //SET_BIT(PORTB, PB3);
    //CLEAR_BIT(PORTB, PB3);
}
