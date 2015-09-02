/*
 * GccApplication4.cpp
 *
 * Created: 5/3/2015 1:17:27 AM
 *  Author: Garrett
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "libs/Neopixel.h"
#include "controller.h"
#include "statemachine.c"

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN)))
#define SET_BIT(TGT, PIN) do { TGT |= (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN) do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^= (1 << (PIN)); } while(0)

uint8_t state_data[] = {
    // One animation, state, and exit
    1, 1, 1,
    // Animation 0, white to black
    0, 0, 0, 
    255, 255, 255,
    INTER_LERP,
    5,
    LOOP_BOUNCE,
    // Exit 0 is impossible
    0,
    SELECTION_ANY,
    0, 0,
    0,
    DIRECTION_NEUTRAL,
    DIRECTION_NEUTRAL,
    0, 0,
    0, 0,
    0,
    // State 0 just loops forever
    0,
    1,
    // Exit arrays
    0   // State 0 links to exit 0
};

void setup_pins(void) {
	CLEAR_BIT(DDRB, PB1);		// Set PB1(AIN1) as input, GCN data signal
	SET_BIT(PORTB, PB1);		// Enable pull-up resistor on PB1
	SET_BIT(DDRB, PB3);		// Set PB3 as output, debug LED
}

void setup_comparator(void) {
	SET_BIT(ACSR, ACBG);		// Enable 1.1V positive input reference voltage
}

void setup_timer0(void) {
	SET_BIT(TCCR0B, CS00);
}

uint8_t wait_amount = 16;

bool getMessage(uint8_t message_buffer[]) {
	// Zero out input array
	for(int i = 0; i < 12; ++i) {
		message_buffer[i] = 0x00;
	}

	// Wait for first bit
	while(!GET_BIT(ACSR, ACO)) {}
	while(GET_BIT(ACSR, ACO)) {}

	for(uint8_t cur_byte = 0; cur_byte < 11; cur_byte++) {
		for(uint8_t bitmask = 0x80; bitmask; bitmask >>= 1) {
			// Reset timer
			TCNT0 = 0;
			// Wait for signal to go low
			while(!GET_BIT(ACSR, ACO)) {
				if(TCNT0 >= 240)	// Timeout
					return false;
			}

			// Reset timer and wait for signal's critical point
			TCNT0 = 0;
			while(TCNT0 <= wait_amount) {}

			// Check if signal is high
			if(!GET_BIT(ACSR, ACO)) {
				message_buffer[cur_byte] |= bitmask;
			}

			// Make sure the signal is high before looping
			while(GET_BIT(ACSR, ACO)) {
				if(TCNT0 >= 240)
					return false;	// Timeout
			}

			// Adjust wait time to be a half-period
			wait_amount = TCNT0 / 2;
		}
	}
	return true;
}

int main(void)
{
	setup_pins();
	setup_comparator();
	setup_timer0();

	ledsetup();

	uint8_t message_buffer[12] = {0};
	Controller *controller = (Controller*)message_buffer;

	Machine state_machine = Machine_deserialize(state_data);

	for(uint8_t i = 0; i < 50; i++) {
	}

	while(1)
	{
		// Wait until we have a valid message
		while(!getMessage(message_buffer)) {}

		// Advance through the state machine and show the color
		Machine_advance(&state_machine, controller);
		Color color = Machine_color(&state_machine);
		showColor(color.r, color.g, color.b, 8);

		_delay_us(2000);		// Wait for the 2nd paired request to pass
	}
}
