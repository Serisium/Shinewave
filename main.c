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
    SET_BIT(DDRB, PB0);		    // Set PB0 as output, debug LED
}

void setup_comparator(void) {
    SET_BIT(ACSR, ACBG);		// Enable 1.1V positive input reference voltage
}

void setup_timer0(void) {
    SET_BIT(TCCR0B, CS00);
}

uint8_t wait_amount = 16;

void init_controller(void) {
    SET_BIT(PORTB, PIN_GC);         // Set positive output on PB1
    SET_BIT(DDRB, PIN_GC);          // Set PB1 as output
    SET_BIT(PORTB, PB0);            // Start debug output

    // Send controller init message (000000001)
    for(uint8_t bit = 0; bit < 8; bit++) {
        SEND_0();
    }
    SEND_1();

    CLEAR_BIT(DDRB, PIN_GC);        // Set PB1 as input
    CLEAR_BIT(PORTB, PB0);          // Stop debug output
}

bool request_message(uint8_t message_buffer[]) {
    SET_BIT(PORTB, PIN_GC);         // Set positive output on PB1
    SET_BIT(DDRB, PIN_GC);          // Set PB1 as output
    SET_BIT(PORTB, PB0);            // Start debug output

    // Send controller data request
    SEND_0(); SEND_1(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); 
    SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_1(); SEND_1(); 
    SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_0(); SEND_1(); SEND_0(); 
    SEND_0();

    CLEAR_BIT(DDRB, PIN_GC);        // Set PB1 as input
    CLEAR_BIT(PORTB, PB0);          // Stop debug output

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
                SET_BIT(PORTB, PB0);
                message_buffer[cur_byte] |= bitmask;
                CLEAR_BIT(PORTB, PB0);
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
    setup_comparator();
    setup_timer0();
    setup_usb();
    init_controller();

    uint8_t message_buffer[8] = {0};
    Controller *controller = (Controller*)message_buffer;

    while(1) {
        // Zero out input array
        for(uint8_t i = 0; i < 8; ++i) {
            message_buffer[i] = 0x00;
        }

        if(CONTROLLER_B(*controller))
            SET_BIT(PORTB, PB0);
        else
            CLEAR_BIT(PORTB, PB0);

        usbPoll();
        wdt_reset();

        if(usbInterruptIsReady()) {
            // Try to grab the controller state
            while(!request_message(message_buffer)) {}

            build_report(controller, reportBuffer);

            usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
        }
    }
}
