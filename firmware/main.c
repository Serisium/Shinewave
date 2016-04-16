#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <util/delay.h>

#include "controller.h"
#include "libs/Neopixel.h"
#include "lookup.h"

#define DEBUG_MATCH 0   // Enable PIN_TIMER toggle on timer match
#define GCN_RETRY_LIMIT 5   // Number of times to retry the GCN signal line

#define PIN_DEBUG PA2
#define PIN_GC    PA6   // Needs to be connected to (DI)
//#define PIN_TIMER PA7   // Needs to be connected to (OC0B). Displays compare match toggles if DEBUG_MATCH is set

#define GET_BIT(TGT, PIN)    ((TGT) & (1 << (PIN)))
#define SET_BIT(TGT, PIN)    do { TGT |=  (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN)  do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^=  (1 << (PIN)); } while(0)

#define SEND_ZERO()        do { CLEAR_BIT(PORTA, PIN_GC); _delay_us(3); SET_BIT(PORTA, PIN_GC); _delay_us(1); } while(0)
#define SEND_ONE()         do { CLEAR_BIT(PORTA, PIN_GC); _delay_us(1); SET_BIT(PORTA, PIN_GC); _delay_us(3); } while(0)

void setup_pins(void) {
    CLEAR_BIT(DDRA, PIN_GC);		// Set PIN_GC as input, GCN data signal
    SET_BIT(PORTA, PIN_GC);		    // Enable pull-up resistor on PIN_GC
    SET_BIT(DDRA, PIN_DEBUG);       // Set PIN_DEBUG as output for debugging
    SET_BIT(DDRA, PA1);             // Set the LED pin as output

    // Ensure that USB pins are inputs with pullup resistor disabled
    CLEAR_BIT(DDRB, PB2);
    CLEAR_BIT(DDRA, PA3);
    CLEAR_BIT(DDRA, PA7);
    CLEAR_BIT(PORTB, PB2);
    CLEAR_BIT(PORTA, PA3);
    CLEAR_BIT(PORTA, PA7);

    #if DEBUG_MATCH == 1
    SET_BIT(DDRA, PIN_TIMER);       // Set PIN_TIMER as output for compare matches
    #endif
    ledsetup();
}

void enable_timer0(void) {
    TCNT0 = 0;
    TIFR0 = 0xff;                   // Reset interrupt flags

    #if DEBUG_MATCH == 1
    // Set 0C0B(PA7) to toggle on match(COM0B=01)
    SET_BIT(TCCR0A, COM0B0);
    #endif

    // Enable Timer/Counter0 module at no prescaler
    SET_BIT(TCCR0B, CS00);
}

void disable_timer0(void) {
    #if DEBUG_MATCH == 1
    // Set 0C0B(PA7) to toggle on match(COM0B=01)
    CLEAR_BIT(TCCR0A, COM0B0);
    #endif

    // Disable Timer/Counter0 module
    CLEAR_BIT(TCCR0B, CS00);
}

// Set Timer/Counter0 in normal mode(WGM=000)
void setup_timer0(void) {
    disable_timer0();

    // Set compare match to signal critical point(2us)
    OCR0A = 2e-6 * F_CPU;   // Triggers USI clock source

    #if DEBUG_MATCH == 1
    OCR0B = 2e-6 * F_CPU;   // Triggers debug toggle
    #endif
}

void enable_usi(void) {
    // Enable Universal Serial Module in Two-wire mode(USIWM=10)
    SET_BIT(USICR, USIWM1);
}

void disable_usi(void) {
    // Disable Universal Serial Module
    SET_BIT(USICR, USIWM1);
}

void setup_usi(void) {
    // Set clock source to Timer/Counter0 Compare Match(USICS=01)
    SET_BIT(USICR, USICS0);
}

void init_controller(void) {
    SET_BIT(PORTA, PIN_GC);         // Set positive output on PIN_GC
    SET_BIT(DDRA, PIN_GC);          // Set PIN_GC as output

    // Send controller init message (000000001)
    for(uint8_t bit = 0; bit < 8; bit++) {
        SEND_ZERO();
    }
    SEND_ONE();

    CLEAR_BIT(DDRA, PIN_GC);        // Set PIN_GC as input
}

uint8_t request_message(uint8_t *message_buffer) {
    uint8_t cur_byte = 0;

    USISR = 0b11100111;             // Reset USI Interrupt flags and set timer value to 8
    disable_usi();
    disable_timer0();

    asm("nop; nop; nop;");

    //SET_BIT(DDRA, PIN_GC);          // Set PIN_GC as output
    //CLEAR_BIT(PORTA, PIN_GC);

    // Send controller data request
    //SEND_ZERO(); SEND_ONE();  SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO();
    //SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ONE();  SEND_ONE();
    //SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ONE();  SEND_ZERO();
    //SEND_ZERO();

    //SET_BIT(PORTA, PIN_GC);
    //CLEAR_BIT(DDRA, PIN_GC);        // Set PIN_GC as input
    
    while(GET_BIT(PINA, PIN_GC)) {}

    // Start reading the message
    enable_usi();
    enable_timer0();
    while(1) {
        // Wait for signal to go low
        while(GET_BIT(PINA, PIN_GC)) {
            // Catch a timer overflow as an exit condition
            // This occurs if the signal is high for > 255 cycles
            if(GET_BIT(TIFR0, TOV0)) {
                // Exit condition
                disable_usi();

                // Wait for the timer to overflow and loop once, fixes 'every other' corruption
                while(TCNT0 < 24) {}

                // Check if there's no signal on the line
                if(cur_byte == 0) {
                    SET_BIT(PORTA, PIN_DEBUG);
                    CLEAR_BIT(PORTA, PIN_DEBUG);
                    return 0;
                }

                disable_timer0();
                return 1;
            }
        }
        // Reset Timer0, a little higher than 0 to account for polling delay
        TCNT0 = 5;

        if(GET_BIT(USISR, USIOIF)) {
            // Skip the counter to 8 of 16
            SET_BIT(USISR, USICNT3);

            // Store the byte from the serial buffer
            message_buffer[cur_byte] = USIBR;
            cur_byte++;

            // Toggle the debug pin
            SET_BIT(PORTA, PIN_DEBUG);
            CLEAR_BIT(PORTA, PIN_DEBUG);

            // Clear the overflow counter
            SET_BIT(USISR, USIOIF);
        }

        // Make sure signal is high before looping
        // Hardware should pull up DIN or risk an infinite loop
        while(!GET_BIT(PINA, PIN_GC)) {}
    }

    // Unreachable code
    return 0;
}

int main(void)
{
    setup_pins();
    setup_timer0();
    setup_usi();
    //setup_usb();
    //init_controller();

    ledsetup();

    uint8_t message_buffer[11] = {0};
    Controller *controller = (Controller*)message_buffer;

    while(1) {
        //for(uint8_t i = 255; i; --i) {
            //showColor(i, 0, 0, 8);
            //_delay_ms(5);
        //}
        
        //showColor(LOOKUP(255), LOOKUP(80), LOOKUP(150), 5);

        // Zero out input array
        //for(uint8_t i = 0; i < 8; ++i) {
            //message_buffer[i] = 0xf0;
        //}

        // Try to grab the controller state
        if(request_message(message_buffer)) {
            if(CONTROLLER_B(*controller)) {
                showColor(LOOKUP(255), 0, 0, 5);
            } else {
                showColor(LOOKUP(255), LOOKUP(80), LOOKUP(150), 5);
            }
        }
    }
}
