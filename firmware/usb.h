#ifndef _USB_H
#define _USB_H

#include <stdint.h>

#include "usbdrv/usbdrv.h"
#include "controller.h"

typedef struct {
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
} report_t;

typedef enum {
    LEFT_CONTROL,
    LEFT_SHIFT,
    LEFT_ALT,
    LEFT_GUI,
    RIGHT_CONTROL,
    RIGHT_SHIFT,
    RIGHT_ALT,
    RIGHT_GUI
} KeyboardModifier;

void build_report(Keyboard *keyboard, report_t *report);

extern report_t reportBuffer;

#endif
