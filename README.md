# Overview

Shinewave is a program for the ATtiny85 microcontroller that allows it to decode a live GameCube controller signal and reactively power RGB LEDs based on button inputs.

[More information](http://electricexploits.net/shinewave/)

[Video](https://www.youtube.com/watch?v=1U4EOI_aFdc)

## Installation & Usage

Shinewave has been written on a Linux system and requires the full avr-gcc toolchain to make.

In Debian, the toolchain can be set up with the following commands.

	sudo apt-get install gcc-avr avr-libc avrdude

Then, download the repository to your local machine. 

	git clone git@github.com:GGreenwood/Shinewave.git
    git checkout t85

To build the project, just run `make`, or `make flash` to build and program. You'll probably need to chance the `PROGRAMMER` line in `Makefile` in order to program it directly.

## Wiring

![Wiring](/images/wiring.png)

## Dependencies

This project depends on one library

* Josh Levine's simple [NeoPixel Demo](https://github.com/bigjosh/SimpleNeoPixelDemo) library
