#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
//#include "libs/Neopixel.h"
#include "libs/delay_x.h"
#include "controller.h"
//#include "statemachine.c"
#include "usb.c"

#define PIN_DEBUG PA1
#define PIN_GC PA2

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

void setup_comparator(void) {
    SET_BIT(ACSR, ACBG);		    // Enable 1.1V positive input reference voltage
}

void setup_timer0(void) {
    OCR0A = 0;                      // Set Timer/Counter0 to hold at 0
    SET_BIT(TCCR0A, WGM00);         // Set Timer/Counter0 in Fast PWM mode(WGM=111)
    SET_BIT(TCCR0A, WGM01);
    SET_BIT(TCCR0B, WGM02);
    SET_BIT(TCCR0B, CS00);          // Enable Timer/Counter0 module at 12MHz
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
    for(uint8_t offset = 0; offset < 8; ++offset) {
        for(uint8_t bitmask = 0x80; bitmask; bitmask >>= 1) {
            // Reset timer
            TCNT0 = 1;
            // Wait for signal to go low
            while(!GET_BIT(ACSR, ACO)) {
                if(TCNT0 == 0)	// Timeout
                    return 0;
            }

            // Reset timer0 and wait for signal's critical point
            __asm__("nop;nop;nop;nop;");
            __asm__("nop;nop;nop;nop;");
            __asm__("nop;");

            // Check if signal is high
            message_buffer[offset] |= !GET_BIT(ACSR, ACO) ? bitmask : 0;

            // Make sure the signal is high before looping
            while(GET_BIT(ACSR, ACO)) {}

            // ADDING MORE THAN TWO INSTRUCTIONS HERE WILL BREAK EVERYTHING
        }
    }
    return 1;
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
    //init_controller();

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
