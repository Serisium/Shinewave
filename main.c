/*
 * GccApplication4.cpp
 *
 * Created: 5/3/2015 1:17:27 AM
 *  Author: Garrett
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "libs/Neopixel.h"
#include "controller.h"
#include "statemachine.c"
#include "usb.c"

#define PIN_GC PB1

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN))) 
#define SET_BIT(TGT, PIN) do { TGT |= (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN) do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^= (1 << (PIN)); } while(0)

#define TIMER_DELAY(CYCLES) do { TCNT0 = 0; while(TCNT0 <= CYCLES) {} } while (0)
#define SEND_0()    do { CLEAR_BIT(PORTB, PIN_GC); TIMER_DELAY(40); SET_BIT(PORTB, PIN_GC); TIMER_DELAY(4); } while (0)
#define SEND_1()    do { CLEAR_BIT(PORTB, PIN_GC); TIMER_DELAY(4); SET_BIT(PORTB, PIN_GC); TIMER_DELAY(40); } while (0)

/*
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
*/

void setup_pins(void) {
    CLEAR_BIT(DDRB, PIN_GC);		// Set PB1(AIN1) as input, GCN data signal
    SET_BIT(PORTB, PIN_GC);		// Enable pull-up resistor on PB1
    SET_BIT(DDRB, PB3);		    // Set PB3 as output, debug LED
}

void setup_comparator(void) {
    SET_BIT(ACSR, ACBG);		// Enable 1.1V positive input reference voltage
}

void setup_timer0(void) {
    SET_BIT(TCCR0B, CS00);
}

uint8_t wait_amount = 16;

void initController(void) {
    SET_BIT(PORTB, PIN_GC);         // Set positive output on PB1
    SET_BIT(DDRB, PIN_GC);          // Set PB1 as output
    SET_BIT(PORTB, PB3);            // Start debug output

    // Send controller init message (000000001)
    for(uint8_t bit = 0; bit < 8; bit++) {
        SEND_0();
    }
    SEND_1();

    CLEAR_BIT(DDRB, PIN_GC);        // Set PB1 as input
    CLEAR_BIT(PORTB, PB3);          // Stop debug output
}

bool requestMessage(uint8_t message_buffer[]) {
    SET_BIT(PORTB, PIN_GC);         // Set positive output on PB1
    SET_BIT(DDRB, PIN_GC);          // Set PB1 as output
    SET_BIT(PORTB, PB3);            // Start debug output

    // Send controller data request
    SEND_0(); SEND_1(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); 
    SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_1(); SEND_1(); 
    SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_1(); SEND_0(); 
    SEND_0();

    CLEAR_BIT(DDRB, PIN_GC);        // Set PB1 as input
    CLEAR_BIT(PORTB, PB3);          // Stop debug output

    // Start reading the message
    for(uint8_t cur_byte = 0; cur_byte < 8; cur_byte++) {
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
                SET_BIT(PORTB, PB3);
                message_buffer[cur_byte] |= bitmask;
                CLEAR_BIT(PORTB, PB3);
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

bool getMessage(uint8_t message_buffer[], bool fullMessage) {
    uint8_t numBytes;
    if(fullMessage) {
        numBytes = 11;
        // Wait for the first bit
        while(!GET_BIT(ACSR, ACO)) {
            wdt_reset();
            usbPoll();
        }
        while(GET_BIT(ACSR, ACO)) {}
    } else {
        numBytes = 8;
    }

    // Start reading the message
    for(uint8_t cur_byte = 0; cur_byte < numBytes; cur_byte++) {
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
                SET_BIT(PORTB, PB3);
                message_buffer[cur_byte] |= bitmask;
                CLEAR_BIT(PORTB, PB3);
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
    //sei();
    return true;
}

int main(void)
{
    setup_pins();
    setup_comparator();
    setup_timer0();

    uint8_t message_buffer[8] = {0};
    Controller *controller = (Controller*)message_buffer;

    initController();

    while(1) {
        // Zero out input array
        for(int i = 0; i < 8; ++i) {
            message_buffer[i] = 0x00;
        }

        while(!requestMessage(message_buffer)) {}

        if(CONTROLLER_B(*controller))
            SET_BIT(PORTB, PB0);
        else
            CLEAR_BIT(PORTB, PB0);

        _delay_ms(15);
    }

    /*
    ledsetup();

    uint8_t message_buffer[12] = {0};
    Controller *controller = (Controller*)message_buffer;

    Machine state_machine = Machine_deserialize(state_data);

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

    while(1)
    {
        // Wait until we have a valid message
        while(!getMessage(message_buffer)) {}

        wdt_reset();
        usbPoll();
        if(usbInterruptIsReady()) {
            reportBuffer.dx = 10;
            usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
        }
        

        // Advance through the state machine and show the color

        if(CONTROLLER_B(*controller)) {
            showColor(255, 0, 0, 8);
        } else {
            showColor(controller->joy_y, controller->c_y, controller->analog_l, controller->analog_r / 32);
        }
        

        _delay_us(2000);		// Wait for the 2nd paired request to pass
    }
    */
}

