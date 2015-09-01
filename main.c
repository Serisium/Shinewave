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
#include "Animation.h"
#include "Controller.h"

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN)))
#define SET_BIT(TGT, PIN) do { TGT |= (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN) do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^= (1 << (PIN)); } while(0)

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
			while(GET_BIT(ACSR, ACO)) {}

			// Adjust wait time to be period/2
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
	initAnimation();

	uint8_t message_buffer[12] = {0};
	Controller *controller = (Controller*)message_buffer;

	while(1)
	{
		while(!getMessage(message_buffer)) {}
		//nextFrame(message_buffer);
		// Test if X is high
		if(CONTROLLER_X(*controller))
			showColor(0, 255, 0, 8);
		else
			showColor(255, 0, 0, 8);

		//showColor(message_buffer[5], message_buffer[6], message_buffer[7], message_buffer[8]/32);

		_delay_us(2000);		// Wait for the 2nd paired request to pass
	}
}
