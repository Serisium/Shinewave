#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
//#include "libs/Neopixel.h"
#include "controller.h"
//#include "statemachine.c"
#include "usb.h"

#define PIN_DEBUG PA5
#define PIN_GC    PA6   // Needs to be connected to (DI)
#define PIN_TIMER PA7   // Needs to be connected to (OC0B)

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN))) 
#define SET_BIT(TGT, PIN)    do { TGT |=  (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN)  do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^=  (1 << (PIN)); } while(0)

#define SEND_ZERO()        do { CLEAR_BIT(PORTA, PIN_GC); _delay_us(3); SET_BIT(PORTA, PIN_GC); _delay_us(1); } while(0)
#define SEND_ONE()         do { CLEAR_BIT(PORTA, PIN_GC); _delay_us(1); SET_BIT(PORTA, PIN_GC); _delay_us(3); } while(0)

void setup_pins(void) {
    CLEAR_BIT(DDRA, PIN_GC);		// Set PIN_GC as input, GCN data signal
    SET_BIT(PORTA, PIN_GC);		    // Enable pull-up resistor on PIN_GC
    SET_BIT(DDRA, PIN_DEBUG);       // Set PIN_DEBUG as output for debugging
    SET_BIT(DDRA, PIN_TIMER);       // Set PIN_TIMER as output for compare matches
}

void enable_timer0(void) {
    TCNT0 = 0;
    TIFR0 = 0xff;                   // Reset interrupt flags
    SET_BIT(TCCR0B, CS00);          // Enable Timer/Counter0 module at no prescaler
}

void disable_timer0(void) {
    CLEAR_BIT(TCCR0B, CS00);        // Disable Timer/Counter0 module
}

// Set Timer/Counter0 in normal mode(WGM=000)
void setup_timer0(void) {
    disable_timer0();

    // Set 0C0B(PA7) to toggle on match(COM0B=01)
    SET_BIT(TCCR0A, COM0B0);

    // Set compare match to signal critical point(2us)
    OCR0A = 2e-6 * F_CPU;   // Triggers USI clock source
    OCR0B = 2e-6 * F_CPU;   // Triggers debug toggle
}

void enable_usi(void) {
    // Enable Universal Serial Module in Two-wire mode(USIWM=10)
    SET_BIT(USICR, USIWM1);
}

void disable_usi(void) {
    // Disable Universal Serial Module in
    SET_BIT(USICR, USIWM1);
}

void setup_usi(void) {
    // Set clock source to Timer/Counter0 Compare Match(USICS=01)
    SET_BIT(USICR, USICS0);
}

void setup_usb(void) {
    cli();
    wdt_enable(WDTO_1S);    // enable 1s watchdog timer
    usbInit();

    usbDeviceDisconnect();  // enforce re-enumeration
    uint8_t i;
    for(i = 0; i<250; i++) {
        wdt_reset();
        _delay_ms(2);
    }
    usbDeviceConnect();
    sei();
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

    USISR = 0b11101000;             // Reset USI Interrupt flags and set timer value to 8

    SET_BIT(DDRA, PIN_GC);          // Set PIN_GC as output
    CLEAR_BIT(PORTA, PIN_GC);

    // Send controller data request
    SEND_ZERO(); SEND_ONE();  SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); 
    SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ONE();  SEND_ONE();
    SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ZERO(); SEND_ONE();  SEND_ZERO(); 
    SEND_ZERO();

    SET_BIT(PORTA, PIN_GC);
    CLEAR_BIT(DDRA, PIN_GC);        // Set PIN_GC as input

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
                while(TCNT0 < 24) {}
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
        // Hardware should pull up AIN1 or risk an infinite loop
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
    setup_usb();
    init_controller();

    uint8_t message_buffer[8] = {0};
    Controller *controller = (Controller*)message_buffer;

    while(1) {
        usbPoll();
        wdt_reset();

        if(usbInterruptIsReady()) {
            // Zero out input array
            for(uint8_t i = 0; i < 8; ++i) {
                message_buffer[i] = 0x00;
            }
            // Try to grab the controller state
            if(request_message(message_buffer)) {
                build_report(controller, reportBuffer);
                usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
                if(reportBuffer.rx == 255) {
                    _delay_us(1);
                }
            }
        }
    }
}
