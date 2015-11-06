
/*
   This is an example of how simple driving a Neopixel can be
   This code is optimized for understandability and changability rather than raw speed
   More info at http://wp.josh.com/2014/05/11/ws2812-neopixels-made-easy/
 */


// Actually send a bit to the string. We must to drop to asm to enusre that the complier does
// not reorder things and make it so the delay happens in the wrong place.

void sendBit( bool bitVal ) {

	if (  bitVal ) {				// 0 bit

		asm volatile (
				"sbi %[port], %[bit] \n\t"				// Set the output bit
				".rept %[onCycles] \n\t"                                // Execute NOPs to delay exactly the specified number of cycles
				"nop \n\t"
				".endr \n\t"
				"cbi %[port], %[bit] \n\t"                              // Clear the output bit
				".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
				"nop \n\t"
				".endr \n\t"
				::
				[port]		"I" (_SFR_IO_ADDR(PIXEL_PORT)),
				[bit]		"I" (PIXEL_BIT),
				[onCycles]	"I" (NS_TO_CYCLES(T1H) - 2),		// 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
				[offCycles] 	"I" (NS_TO_CYCLES(T1L) - 2)			// Minimum interbit delay. Note that we probably don't need this at all since the loop overhead will be enough, but here for correctness

			     );

	} else {					// 1 bit

		// **************************************************************************
		// This line is really the only tight goldilocks timing in the whole program!
		// **************************************************************************


		asm volatile (
				"sbi %[port], %[bit] \n\t"				// Set the output bit
				".rept %[onCycles] \n\t"				// Now timing actually matters. The 0-bit must be long enough to be detected but not too long or it will be a 1-bit
				"nop \n\t"                                              // Execute NOPs to delay exactly the specified number of cycles
				".endr \n\t"
				"cbi %[port], %[bit] \n\t"                              // Clear the output bit
				".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
				"nop \n\t"
				".endr \n\t"
				::
				[port]		"I" (_SFR_IO_ADDR(PIXEL_PORT)),
				[bit]		"I" (PIXEL_BIT),
				[onCycles]	"I" (NS_TO_CYCLES(T0H) - 2),
				[offCycles]	"I" (NS_TO_CYCLES(T0L) - 2)

			     );

	}

	// Note that the inter-bit gap can be as long as you want as long as it doesn't exceed the 5us reset timeout (which is A long time)
	// Here I have been generous and not tried to squeeze the gap tight but instead erred on the side of lots of extra time.
	// This has thenice side effect of avoid glitches on very long strings becuase 


}  


void sendByte( unsigned char byte ) {

	for( unsigned char bit = 0 ; bit < 8 ; bit++ ) {

		sendBit( GET_BIT( byte , 7 ) );                // Neopixel wants bit in highest-to-lowest order
		// so send highest bit (bit #7 in an 8-bit byte since they start at 0)
		byte <<= 1;                                    // and then shift left so bit 6 moves into 7, 5 moves into 6, etc

	}           
} 

void delay_ms( int ms )
{
	for (int i = 0; i < ms; i++)
	{
		_delay_ms(1);
	}
}

/*

   The following three functions are the public API:

   ledSetup() - set up the pin that is connected to the string. Call once at the begining of the program.  
   sendPixel( r g , b ) - send a single pixel to the string. Call this once for each pixel in a frame.
   show() - show the recently sent pixel on the LEDs . Call once per frame. 

 */


// Set the specified pin up as digital out

void ledsetup() {

	SET_BIT( PIXEL_DDR , PIXEL_BIT );

}

void sendPixel( unsigned char r, unsigned char g , unsigned char b )  {  

	sendByte(g);          // Neopixel wants colors in green then red then blue order
	sendByte(r);
	sendByte(b);

}


// Just wait long enough without sending any bots to cause the pixels to latch and display the last sent frame

void show() {
	_delay_us( (RES / 1000UL) + 1);				// Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}


/*

   That is the whole API. What follows are some demo functions rewriten from the AdaFruit strandtest code...

https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest/strandtest.ino

Note that we always turn off interrupts while we are sending pixels becuase an interupt
could happen just when we were in the middle of somehting time sensitive.

If we wanted to minimize the time interrupts were off, we could instead 
could get away with only turning off interrupts just for the very brief moment 
when we are actually sending a 0 bit (~1us), as long as we were sure that the total time 
taken by any interrupts + the time in our pixel generation code never exceeded the reset time (5us).

 */


// Display a single color on the whole string

void showColor( unsigned char r , unsigned char g , unsigned char b , unsigned char brightness) { 
	r = (uint16_t) r * brightness / 8;
	g = (uint16_t) g * brightness / 8;
	b = (uint16_t) b * brightness / 8;
	for( int p=0; p<PIXELS; p++ ) {
		sendPixel( r , g , b );
	}
	show();
}
