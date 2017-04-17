#ifndef _CONTROL_H
#define _CONTROL_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define GET_BIT(TGT, PIN) ((TGT) & (1 << (PIN)))
#define SET_BIT(TGT, PIN) do { TGT |= (1 << (PIN)); } while(0)
#define CLEAR_BIT(TGT, PIN) do { TGT &= ~(1 << (PIN)); } while(0)
#define TOGGLE_BIT(TGT, PIN) do { TGT ^= (1 << (PIN)); } while(0)

void setup_pins(void);
void setup_comparator_interrupt(void);
void clear_interrupts(void);
void setup_timer0(void);
void enable_timer0(void);
void disable_timer0(void);

#endif
