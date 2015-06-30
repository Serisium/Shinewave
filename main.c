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
//#include "Neopixel.cpp"
//#include "Controller.h"

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN)))
#define SET_BIT(TGT, PIN) do { TGT |= (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN) do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^= (1 << (PIN)); } while(0)

void setup_pins(void) {
	CLEAR_BIT(DDRB, PB1);		// Set PB1(AIN1) as input, GCN data signal
	SET_BIT(PORTB, PB1);		// Enable pull-up resistor on PB1
	SET_BIT(DDRB, PB3);		// Set PB3 as output, debug LED
}

void setup_comparator_interrupt(void) {
	SET_BIT(ACSR, ACBG);		// Enable 1.1V positive input reference voltage

	SET_BIT(ACSR, ACIS0);		// Enable rising edge interrupts (we are connected to
	SET_BIT(ACSR, ACIS1);		//  the negative input, so the signal is inverted)

	SET_BIT(ACSR, ACI);		// Clear any pending interrupts

	SET_BIT(ACSR, ACIE);		// Enable Analog Comparator interrupts
}

void clear_interrupts(void) {
	SET_BIT(ACSR, ACI);		// Clear pending Analog Comparator interrupts
	SET_BIT(TIFR, TOV0);		// Clear pending Timer0 overflow interrupts
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
//static volatile Controller *controller = (Controller*)byte_buffer;

volatile uint8_t byte_index = 0;
volatile uint8_t bitmask = 0x01;

volatile uint8_t is_done = 0;

int main(void)
{
	setup_pins();
	setup_comparator_interrupt();
	setup_timer0();

	ledsetup();

	sei();

	while(1)
	{
		if(is_done == 1) {
			cli();
			is_done = 0;
			nextFrame(byte_buffer);

			/*
			if(byte_buffer[4] & 0x02 && byte_buffer[7] < 80) {
			//if(controller->console_message[1] == 0x80) {
			//if(controller->joy_y < 80) {
				SET_BIT(PORTB, PB3);
				//detonate(0, 0, 255, 100);
			} else {
				CLEAR_BIT(PORTB, PB3);
			}
			*/

			for(int i = 0; i < 12; ++i) {
				byte_buffer[i] = 0x00;
			}

			_delay_us(2000);		// Wait for the 2nd paired request to pass
			clear_interrupts();
			sei();		
		}
	}
}

ISR(TIM0_OVF_vect) {
	disable_timer0();
	is_done = 1;

	bitmask = 0x01;
	byte_index = 0;
	//SET_BIT(PORTB, PB3);
	//CLEAR_BIT(PORTB, PB3);
}
