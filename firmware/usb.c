#include "usb.h"

#include <avr/pgmspace.h>

#include "controller.h"
#include "usbdrv/usbdrv.h"

report_t reportBuffer;

PROGMEM const char usbHidReportDescriptor[48] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Game Pad)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x10,                    //   USAGE_MAXIMUM (Button 16)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x10,                    //   REPORT_COUNT (16)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //   USAGE (X)
    0x09, 0x31,                    //   USAGE (Y)
    0x09, 0x32,                    //   USAGE (Z)
    0x09, 0x33,                    //   USAGE (Rx)
    0x09, 0x34,                    //   USAGE (Ry)
    0x09, 0x35,                    //   USAGE (Rz)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0xc0                           //   END_COLLECTION
};

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (void *)data;

    // The following requests are never used. But since they are required by
    // the specification, we implement them in this example.
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        if(rq->bRequest == USBRQ_HID_GET_REPORT) {
            // wValue: ReportType (highbyte), ReportID (lowbyte)
            usbMsgPtr = (void *)&reportBuffer; // we only have this one
            return sizeof(reportBuffer);
        }
    }
    return 0;
}

void build_report(Controller *controller, report_t *report) {
    if(CONTROLLER_A(*controller))
        report->buttonMask |= (1 << 0);
    else
        report->buttonMask &= ~(1 << 0);

    if(CONTROLLER_X(*controller))
        report->buttonMask |= (1 << 1);
    else
        report->buttonMask &= ~(1 << 1);

    if(CONTROLLER_B(*controller))
        report->buttonMask |= (1 << 2);
    else
        report->buttonMask &= ~(1 << 2);

    if(CONTROLLER_Y(*controller))
        report->buttonMask |= (1 << 3);
    else
        report->buttonMask &= ~(1 << 3);

    if(controller->analog_r > 128)
        report->buttonMask |= (1 << 4);
    else
        report->buttonMask &= ~(1 << 4);

    if(controller->analog_l > 128)
        report->buttonMask |= (1 << 5);
    else
        report->buttonMask &= ~(1 << 5);

    if(CONTROLLER_D_UP(*controller))
        report->buttonMask |= (1 << 6);
    else
        report->buttonMask &= ~(1 << 6);

    if(CONTROLLER_START(*controller))
        report->buttonMask |= (1 << 7);
    else
        report->buttonMask &= ~(1 << 7);

    report->x = controller->joy_x;
    report->y = -(controller->joy_y);
    //report->z = -(controller->analog_r)/2 + 128;
    //report->rx = -(controller->analog_l)/2 + 128;
    report->z = 100;
    report->rx = 128;
    report->ry = controller->c_x;
    report->rz = -(controller->c_y);
}

