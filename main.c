#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "animation/animation.h"
#include "controller.h"
#include "control.h"
#include "TIM0_OVF.h"

volatile uint8_t byte_buffer[20];
static Controller *controller = (Controller*)byte_buffer;

volatile uint8_t bitmask = 0x01;
volatile uint8_t byte_index = 0;

volatile uint8_t is_done = 0;

int main(void)
{
    setup_pins();
    setup_comparator_interrupt();
    setup_timer0();

    ledsetup();
    Animation *animation = malloc(sizeof(Animation));
    init_animation(animation);

    sei();

    while(1)
    {
        if(is_done == 1) {
            cli();
            is_done = 0;
            next_frame(animation, controller);

            for(int i = 0; i < 12; ++i) {
                byte_buffer[i] = 0x00;
            }

            _delay_us(2000);		// Wait for the 2nd paired request to pass
            clear_interrupts();
            sei();		
        }
    }
}
