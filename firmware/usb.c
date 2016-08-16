#include "usb.h"
#include "controller.h"

#define STATE_WAIT 0
#define STATE_SEND_KEY 1
#define STATE_RELEASE_KEY 2

#define NUM_LOCK 1
#define CAPS_LOCK 2
#define SCROLL_LOCK 4

report_t reportBuffer;
volatile static uchar LED_state = 0xff; // received from PC
static uchar idleRate;

PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x06,                    // USAGE (Keyboard)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x75, 0x01,                    //   REPORT_SIZE (1)
	0x95, 0x08,                    //   REPORT_COUNT (8)
	0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)(224)
	0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs) ; Modifier byte
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x81, 0x03,                    //   INPUT (Cnst,Var,Abs) ; Reserved byte
	0x95, 0x05,                    //   REPORT_COUNT (5)
	0x75, 0x01,                    //   REPORT_SIZE (1)
	0x05, 0x08,                    //   USAGE_PAGE (LEDs)
	0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
	0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
	0x91, 0x02,                    //   OUTPUT (Data,Var,Abs) ; LED report
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x03,                    //   REPORT_SIZE (3)
	0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs) ; LED report padding
	0x95, 0x06,                    //   REPORT_COUNT (6)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
	0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))(0)
	0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)(101)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0xc0                           // END_COLLECTION
};

// Even bytes(left) are scan codes recorded from the keyboard controller
// Odd bytes(right) are the HID-compliant keycodes that they map to
//   Chapter 10: http://www.usb.org/developers/hidpage/Hut1_12v2.pdf
#define KEYBOARD_MISC_LOOKUP_LEN 56
PROGMEM const char keyboardMiscLookup[KEYBOARD_MISC_LOOKUP_LEN] = {
    0x61, 0x28,     // Enter
    0x4c, 0x29,     // Esc
    0x50, 0x2a,     // Backspace
    0x51, 0x2b,     // Tab
    0x59, 0x2c,     // Space
    0x34, 0x2d,     // '-', '_'
    0x35, 0x2e,     // '=', '+'
    0x37, 0x2f,     // '[', '{'
    0x38, 0x30,     // ']', '}'
    0x3B, 0x31,     // '\', '|'
    0x39, 0x33,     // ';', ':'
    0x3a, 0x34,     // ''', '"'
    0x4f, 0x35,     // '`', '~'
    0x3c, 0x36,     // ',', '<'
    0x3d, 0x37,     // '.', '>'
    0x3e, 0x38,     // '/', '?'
    0x53, 0x39,     // Caps lock
    0x0a, 0x47,     // Scroll lock
    0x4d, 0x49,     // Insert
    0x06, 0x4a,     // Home
    0x08, 0x4b,     // Page Up
    0x4e, 0x4c,     // Delete
    0x07, 0x4d,     // End
    0x09, 0x4e,     // Page Down
    0x5f, 0x4f,     // Right arrow
    0x5c, 0x50,     // Left arrow
    0x5d, 0x51,     // Down arrow
    0x5e, 0x52,     // Up arrow
};


// Even bytes(left) are scan codes recorded from the keyboard controller
// Odd bytes(right) are the HID-compliant modifier ID's that they map to
//   https://docs.mbed.com/docs/ble-hid/en/latest/api/md_doc_HID.html
#define KEYBOARD_MODIFIER_LOOKUP_LEN 10
PROGMEM const char keyboardModifierLookup[KEYBOARD_MODIFIER_LOOKUP_LEN] = {
    0x56, LEFT_CONTROL,
    0x54, LEFT_SHIFT,
    0x57, LEFT_ALT,
    0x58, LEFT_GUI,
    0x55, RIGHT_SHIFT
};

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data;

	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
		switch(rq->bRequest) {
			case USBRQ_HID_GET_REPORT: // send "no keys pressed" if asked here
				// wValue: ReportType (highbyte), ReportID (lowbyte)
				usbMsgPtr = (void *)&reportBuffer; // we only have this one
				reportBuffer.modifier = 0;
				reportBuffer.keycode[0] = 0;
				return sizeof(reportBuffer);
			case USBRQ_HID_SET_REPORT: // if wLength == 1, should be LED state
				return (rq->wLength.word == 1) ? USB_NO_MSG : 0;
			case USBRQ_HID_GET_IDLE: // send idle rate to PC as required by spec
				usbMsgPtr = &idleRate;
				return 1;
			case USBRQ_HID_SET_IDLE: // save idle rate as required by spec
				idleRate = rq->wValue.bytes[1];
				return 0;
		}
	}

	return 0; // by default don't return any data
}

usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len) {
	if (data[0] == LED_state)
		return 1;
	else
		LED_state = data[0];

	// LED state changed
    /*
	if(LED_state & CAPS_LOCK)
		SET_BIT(PORTA, PIN_DEBUG);		// LED on
	else
		CLEAR_BIT(PORTA, PIN_DEBUG);	// LED off
    */

	return 1; // Data read, not expecting more
}

// Translate ASCII Keyboard codes to HID reports
void build_report(Keyboard *keyboard, report_t *report) {
    // Clear report buffer
    report->modifier = 0;
    report->reserved = 0;
    for(uint8_t i = 0; i < 6; i++) {
        report->keycode[i] = 0;
    }

    // Loop through each button code from the keyboard
    uint8_t buttonIndex;
    for(buttonIndex = 0; buttonIndex < 3; buttonIndex++) {
        uint8_t button = keyboard->button[buttonIndex];
        // Check for a NULL character
        if(button == 0x00) {
            report->keycode[buttonIndex] = 0x00;
            continue;
        }

        // Check for modifiers
        for(uint8_t i = 0; i < KEYBOARD_MODIFIER_LOOKUP_LEN; i += 2) {
            if(button == pgm_read_byte(&(keyboardModifierLookup[i]))) {
                report->modifier |= 1 << pgm_read_byte(&(keyboardModifierLookup[i + 1]));
                break;
            }
        }

        // Between 'a' and '0'
        if(button >= 0x10 && button < 0x34) {
            report->keycode[buttonIndex] = button - 12;
        // Between 'F1' and 'F12'
        } else if(button >= 0x40 && button < 0x4c) {
            report->keycode[buttonIndex] = button - 6;
        } else {
            // Check misc table for other codes
            for(uint8_t i = 0; i < KEYBOARD_MISC_LOOKUP_LEN; i += 2) {
                if(button == pgm_read_byte(&(keyboardMiscLookup[i]))) {
                    report->keycode[buttonIndex] = pgm_read_byte(&(keyboardMiscLookup[i + 1]));
                    break;
                }
            }
        }
    }
}
