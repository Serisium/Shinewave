#include "control.h"

void setup_pins(void) {
    CLEAR_BIT(DDRB, PB1);		// Set PB1(AIN1) as input, GCN data signal
    SET_BIT(PORTB, PB1);		// Enable pull-up resistor on PB1
    SET_BIT(DDRB, PB3);		    // Set PB3 as output, debug LED
}

void setup_comparator_interrupt(void) {
    SET_BIT(ACSR, ACBG);		// Enable 1.1V positive input reference voltage

    SET_BIT(ACSR, ACIS0);		// Enable rising edge interrupts (we are connected to
    SET_BIT(ACSR, ACIS1);		//  the negative input, so the signal is inverted)

    SET_BIT(ACSR, ACI);		    // Clear any pending interrupts

    SET_BIT(ACSR, ACIE);		// Enable Analog Comparator interrupts
}

void clear_interrupts(void) {
    SET_BIT(ACSR, ACI);		    // Clear pending Analog Comparator interrupts
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
