#ifndef _USB_H
#define _USB_H

#include <stdint.h>

#include "controller.h"
#include "usbdrv/usbdrv.h"

typedef struct {
    uint16_t buttonMask;
    uint8_t x, y, z, rx, ry, rz;
} report_t;

void build_report(Controller *controller, report_t *report);

extern report_t reportBuffer;

#endif
