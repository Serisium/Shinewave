#include <avr/io.h>

#define __SFR_OFFSET 0

.extern byte_buffer, byte_index, bitmask

.global ANA_COMP_vect

	ANA_COMP_vect:
		push r31		;Z High register
		push r30		;Z Low register
		push r25		;Working register
		push r24		;Bitmask
		push r23		;Byte to store
		push r22		;Byte index

		out TCNT0, r1				;Reset Timer0

		ldi r25, 0x01
		out TCCR0B, r25				;Enable Timer0 (CS00 = 1)

		lds r24, bitmask
		lds r22, byte_index

		ldi r30, lo8(byte_buffer)		;(r31-r30) is the Z two-byte register
		ldi r31, hi8(byte_buffer)
		add r30, r22				;Add the index offset
		ldd r23, Z+0				;Load the current byte
		
		nop					;Wait a few cycles to make sure 
		nop					; we read at the top of the signal
		nop
		nop
		;sbi PORTB, PB3
		;cbi PORTB, PB3

		sbis ACSR, ACO				;If PB2 is high, then
		or r23, r24				; OR the current byte with the bitmask

		std Z+0, r23				;Store back the current byte

		clc
		ror r24					;Right-shift the bitmask
		brcc done				;If the right shift overflowed, run next_byte, otherwise run done

	next_byte:
		ldi r24, 0x80				;Reset the bitmask

		ldi r25, 0x01
		;cpse					;TODO: Add safety to make sure we don't overwrite past byte_buffer
		add r22, r25				;Increment the byte index

		sts byte_index, r22
		
	done:
		sts bitmask, r24			;Store the modified bitmask

		;sbi PORTB, PB3
		;cbi PORTB, PB3

		pop r22
		pop r23
		pop r24
		pop r25
		pop r30
		pop r31

		reti
