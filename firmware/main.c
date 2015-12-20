#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
//#include "libs/Neopixel.h"
//#include "libs/delay_x.h"
#include "controller.h"
//#include "statemachine.c"
#include "usb.h"

#define PIN_DEBUG PA7
#define PIN_GC PA6

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN))) 
#define SET_BIT(TGT, PIN) do { TGT |= (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN) do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^= (1 << (PIN)); } while(0)

#define SEND_ZERO()        do { CLEAR_BIT(PORTA, PIN_GC); _delay_us(3); SET_BIT(PORTA, PIN_GC); _delay_us(1); } while(0)
#define SEND_ONE()         do { CLEAR_BIT(PORTA, PIN_GC); _delay_us(1); SET_BIT(PORTA, PIN_GC); _delay_us(3); } while(0)

void setup_pins(void) {
    CLEAR_BIT(DDRA, PIN_GC);		// Set PIN_GC as input, GCN data signal
    SET_BIT(PORTA, PIN_GC);		    // Enable pull-up resistor on PIN_GC
    SET_BIT(DDRA, PIN_DEBUG);       // Set PIN_DEBUG as output, debug LED
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
    OCR0B = 2e-6 * F_CPU;
}

void setup_usi(void) {
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

const uint8_t wait_amount = 8;

uint8_t request_message(uint8_t *message_buffer) {
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
    enable_timer0();
    while(1) {
        // Wait for signal to go low
        while(GET_BIT(PINA, PIN_GC)) {
            // Catch a timer overflow as an exit condition
            // This occurs if the signal is high for > 255 cycles
            if(GET_BIT(TIFR0, TOV0)) {
                // Exit condition
                disable_timer0();
                return 1;
            }
        }

        // Reset Timer0, a little higher than 0 to account for polling delay
        TCNT0 = 5;

        // Make sure signal is high before looping
        // Hardware should pull up AIN1 or risk an infinite loop
        while(!GET_BIT(PINA, PIN_GC)) {}
    }

    // Unreachable code
    return 0;
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

int main(void)
{
    setup_pins();
    setup_timer0();
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
                    SET_BIT(PORTA, PIN_DEBUG);
                    _delay_us(1);
                    CLEAR_BIT(PORTA, PIN_DEBUG);
                }
            }
        }
    }
}
