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

void enable_timer0(void) {
	SET_BIT(TCCR0B, CS00);
}

void disable_timer0(void) {
	CLEAR_BIT(TCCR0B, CS00);
}

//static volatile Controller *controller = (Controller*)message_buffer;

volatile uint8_t is_done = 0;

bool getMessage(uint8_t message_buffer[]) {
	TCNT0 = 0;
	TCCR0B = 0;

	// Wait for first bit
	while(!GET_BIT(ACSR, ACO)) {}

	for(uint8_t cur_byte = 0; cur_byte < 11; cur_byte++) {
		for(uint8_t bitmask = 0x80; bitmask; bitmask >>= 1) {
			// Make sure the signal is high before reading
			while(GET_BIT(ACSR, ACO)) {}

			TCCR0B = 1;
			// Wait for signal to go low
			while(!GET_BIT(ACSR, ACO)) {
				if(TCNT0 >= 240)	// Timeout
					return false;
			}

			// Reset timer and wait for signal's critical point
			TCNT0 = 0;
			while(TCNT0 <= 16) {}

			// Check if signal is high
			if(!GET_BIT(ACSR, ACO)) {
				message_buffer[cur_byte] |= bitmask;
			}

			TCCR0B = 0;
			TCNT0 = 0;
		}
	}
	return true;
}

int main(void)
{
	setup_pins();
	setup_comparator();

	ledsetup();
	initAnimation();

	uint8_t message_buffer[12] = {0};

	while(1)
	{
		while(!getMessage(message_buffer)) {}
		//nextFrame(message_buffer);
		// Test if X is high
		if(message_buffer[3] & 0x04) {
			showColor(255, 0, 0, 4);
		} else {
			showColor(0, 255, 0, 4);
		}

		for(int i = 0; i < 12; ++i) {
			message_buffer[i] = 0x00;
		}

		_delay_us(2000);		// Wait for the 2nd paired request to pass
	}
}
