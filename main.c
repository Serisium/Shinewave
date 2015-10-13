#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "libs/Neopixel.h"
#include "controller.h"
#include "statemachine.c"
#include "usb.c"

// GC Signal pin must be connected to OC0A
#define PIN_DEBUG PB0
#define PIN_GC PB1

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN))) 
#define SET_BIT(TGT, PIN) do { TGT |= (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN) do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^= (1 << (PIN)); } while(0)

#define DELAY_OVERHEAD  10

#define DELAY_SHORT     (( 1 * F_CPU / 1e6 ) - 2)   // 1 us in clock cycles 
#define DELAY_LONG      (( 3 * F_CPU / 1e6 ) - 2)   // 3 us in clock cycles

#define DELAY_TOTAL     (( 4 * F_CPU / 1e6 ) - DELAY_OVERHEAD )   // 4 us in clock cycles

#define SEND_0()        do { OCR0B = 0xff - DELAY_SHORT; TCNT0 = 0xff - DELAY_TOTAL; while(TCNT0) {} } while (0)
#define SEND_1()        do { OCR0B = 0xff - DELAY_LONG;  TCNT0 = 0xff - DELAY_TOTAL; while(TCNT0) {} } while (0)

//#define TIMER_DELAY(CYCLES) do { TCNT1 = 0xff - CYCLES; while(TCNT1 != 0) {} } while (0)
//#define SEND_0()        do { OCR0B = DELAY_LONG;  TCNT0 = DELAY_LONG - 1;  TIMER_DELAY(DELAY_TOTAL); } while (0)
//#define SEND_1()        do { OCR0B = DELAY_SHORT; TCNT0 = DELAY_SHORT - 1; TIMER_DELAY(DELAY_TOTAL); } while (0)

void setup_pins(void) {
    CLEAR_BIT(DDRB, PIN_GC);		// Set PIN_GC as input, GCN data signal
    SET_BIT(PORTB, PIN_GC);		    // Enable pull-up resistor on PIN_GC
    SET_BIT(DDRB, PIN_DEBUG);       // Set PIN_DEBUG as output, debug LED
}

void setup_comparator(void) {
    SET_BIT(ACSR, ACBG);		    // Enable 1.1V positive input reference voltage
}

void setup_timer0(void) {
    // Set timer1 as timekeeper for reading messages
    //SET_BIT(TCCR1B, CS10);      

    // Set timer0 in "one-shot" mode for sending messages
    // http://wp.josh.com/2015/03/12/avr-timer-based-one-shot-explained/
    TCCR0B = 0;                     // Disable timer0 until config is done
    TCNT0 = 0;                      // Reset timer0
    OCR0A = 0;
    
    // Set output line on matches, clear at bottom
    SET_BIT(TCCR0A, COM0B0);
    SET_BIT(TCCR0A, COM0B1);

    // Enable Fast PWM Mode
    SET_BIT(TCCR0A, WGM00);
    SET_BIT(TCCR0A, WGM01);
    SET_BIT(TCCR0B, WGM02);
    
    // Start counting(no prescaler)
    SET_BIT(TCCR0B, CS00);

    //OCR1C = 0;
    //SET_BIT(TCCR1, CTC1);   // Hold timer at 0
    SET_BIT(TCCR1, CS10);
}


void init_controller(void) {
    SET_BIT(PORTB, PIN_GC);         // Set positive output on PB0
    SET_BIT(DDRB, PIN_GC);          // Set PB0 as output

    // Send controller init message (000000001)
    for(uint8_t bit = 0; bit < 8; bit++) {
        SEND_0();
    }
    SEND_1();

    CLEAR_BIT(DDRB, PIN_GC);        // Set PB0 as input
}

uint8_t wait_amount = 248;

bool request_message(uint8_t message_buffer[]) {
    //SET_BIT(PORTB, PIN_GC);         // Set positive output on PB0
    SET_BIT(DDRB, PIN_GC);          // Set PB0 as output
    CLEAR_BIT(PORTB, PIN_GC);

    // Send controller data request
    SEND_0(); SEND_1(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); 
    SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_1(); SEND_1(); 
    SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_1(); SEND_0(); 
    SEND_0();

    SET_BIT(PORTB, PIN_GC);
    CLEAR_BIT(DDRB, PIN_GC);        // Set PB0 as input

    // Start reading the message
    for(uint8_t cur_byte = 0; cur_byte < 8; cur_byte++) {
        for(uint8_t bitmask = 0x80; bitmask; bitmask >>= 1) {
            // Reset timer
            TCNT0 = 1;
            // Wait for signal to go low
            while(GET_BIT(PINB, PIN_GC)) {
                if(TCNT0 == 0)	// Timeout
                    return false;
            }

            // Reset timer1 and wait for signal's critical point
            TCNT0 = wait_amount;
            while(TCNT0 != 0) {}

            // Check if signal is high
                SET_BIT(PORTB, PIN_DEBUG);
                CLEAR_BIT(PORTB, PIN_DEBUG);
            if(GET_BIT(PINB, PIN_GC)) {
                message_buffer[cur_byte] |= bitmask;
            }

            // Make sure the signal is high before looping
            TCNT0 = 1;
            while(!GET_BIT(PINB, PIN_GC)) {
                if(TCNT0 == 0)
                    return false;	// Timeout
            }

            // Adjust wait time to be a half-period
            //wait_amount = TCNT0 / 2;
        }
    }
    return true;
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

void build_report(Controller *controller, report_t report) {
    if(CONTROLLER_A(*controller))
        reportBuffer.buttonMask |= (1 << 0);
    else
        reportBuffer.buttonMask &= ~(1 << 0);

    if(CONTROLLER_B(*controller))
        reportBuffer.buttonMask |= (1 << 1);
    else
        reportBuffer.buttonMask &= ~(1 << 1);

    if(CONTROLLER_X(*controller))
        reportBuffer.buttonMask |= (1 << 2);
    else
        reportBuffer.buttonMask &= ~(1 << 2);

    if(CONTROLLER_Y(*controller))
        reportBuffer.buttonMask |= (1 << 3);
    else
        reportBuffer.buttonMask &= ~(1 << 3);

    if(CONTROLLER_START(*controller))
        reportBuffer.buttonMask |= (1 << 7);
    else
        reportBuffer.buttonMask &= ~(1 << 7);

    if(CONTROLLER_D_UP(*controller))
        reportBuffer.buttonMask |= (1 << 6);
    else
        reportBuffer.buttonMask &= ~(1 << 6);

    if(controller->analog_l > 127)
        reportBuffer.buttonMask |= (1 << 4);
    else
        reportBuffer.buttonMask &= ~(1 << 4);

    reportBuffer.x = controller->joy_x;
    reportBuffer.y = -(controller->joy_y);
    reportBuffer.z = controller->analog_l;
    reportBuffer.rx = controller->c_x;
    reportBuffer.ry = controller->c_y;
    reportBuffer.rz = controller->analog_r;
}

int main(void)
{
    setup_pins();
    //setup_comparator();
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
            //SET_BIT(PORTB, PIN_GC);            // Start debug output
            // Try to grab the controller state
            //while(!request_message(message_buffer)) {}
            request_message(message_buffer);
            //SET_BIT(DDRB, PIN_GC);
            //SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_1();
            //CLEAR_BIT(DDRB, PIN_GC);

            build_report(controller, reportBuffer);

            usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
        }
    }
}
