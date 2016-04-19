#ifndef _NEOPIXEL_H
#define _NEOPIXEL_H

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//#include "lookup.h"

// Change this to be at least as long as your pixel string (too long will work fine, just be a little slower)

#define PIXELS 5  // Number of pixels in the string

// These values depend on which pin your string is connected to and what board you are using
// More info on how to find these at http://www.arduino.cc/en/Reference/PortManipulation

// These values are for digital pin 8 on an Arduino Yun or digital pin 12 on a DueMilinove/UNO
// Note that you could also include the DigitalWriteFast header file to not need to to this lookup.

#define PIXEL_PORT  PORTA  // Port of the pin the pixels are connected to
#define PIXEL_DDR   DDRA   // Port of the pin the pixels are connected to
#define PIXEL_BIT   PA1    // Bit of the pin the pixels are connected to

// These are the timing constraints taken mostly from the WS2812 datasheets
// These are chosen to be conservative and avoid problems rather than for maximum throughput

#define T1H  900    // Width of a 1 bit in ns
#define T1L  600    // Width of a 1 bit in ns

#define T0H  400    // Width of a 0 bit in ns
#define T0L  900    // Width of a 0 bit in ns

#define RES 7000    // Width of the low gap between bits to cause a frame to latch

// Here are some convenience defines for using nanoseconds specs to generate actual CPU delays

#define NS_PER_SEC (1000000000L)          // Note that this has to be SIGNED since we want to be able to check for negative values of derivatives

#define CYCLES_PER_SEC (F_CPU)

#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )

#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN)))
#define SET_BIT(TGT, PIN) do { TGT |= (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN) do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^= (1 << (PIN)); } while(0)

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Color;

void sendBit(bool bitVal);
void sendByte(unsigned char byte);
void ledsetup();
void sendPixel(Color col);
void showColor(Color col);

#endif
