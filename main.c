/*
 * GccApplication4.cpp
 *
 * Created: 5/3/2015 1:17:27 AM
 *  Author: Garrett
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Neopixel.h"
#include "Neopixel.cpp"

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN)))
#define SET_BIT(TGT, PIN) do { TGT |= (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN) do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^= (1 << (PIN)); } while(0)

void setup_pins(void) {
	CLEAR_BIT(DDRB, PB2);		// Set PB2 as input
	SET_BIT(PORTB, PB2);		// Enable pull-up resistor on PB2

	SET_BIT(DDRB, PB3);		// Set PB3 as output
}

void setup_external_interrupt(void) {
	SET_BIT(MCUCR, ISC01);		// Generate interrupt on falling edge
	SET_BIT(GIMSK, INT0);		// Enable external interrupts
}

void setup_timer0(void) {
	SET_BIT(TIMSK, TOIE0);		// Enable overflow interrupts
}
void enable_timer0(void) {
	SET_BIT(TCCR0B, CS00);
}
void disable_timer0(void) {
	CLEAR_BIT(TCCR0B, CS00);
}


volatile uint8_t byte_buffer[20];
volatile uint8_t byte_index = 0;
volatile uint8_t bitmask = 0x01;

volatile uint8_t is_done = 0;

int main(void)
{
	setup_pins();
	setup_external_interrupt();
	setup_timer0();

	ledsetup();

	sei();

	while(1)
	{
		if(is_done == 1) {
			cli();
			is_done = 0;

			if(byte_buffer[4] & 0x02 && byte_buffer[7] < 80){
				SET_BIT(PORTB, PB3);
				detonate(0, 0, 255, 100);
			} else {
				CLEAR_BIT(PORTB, PB3);
			}

			for(int i = 0; i < 12; ++i) {
				byte_buffer[i] = 0x00;
			}

			sei();		
		}


	}
}

ISR(TIM0_OVF_vect) {
	disable_timer0();
	is_done = 1;

	bitmask = 0x01;
	byte_index = 0;
}


