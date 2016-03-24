# Overview
Shinewave is a mod for the Nintendo GameCube controller that adds USB joystick functionality, along with LED lighting.

![Controller](/images/controller.jpg)

# Installation & Usage
Shinewave has been written on a Linux system and requires the full avr-gcc toolchain to make.

In Debian, the toolchain can be set up with the following command.

	sudo apt-get install gcc-avr avr-libc avrdude

In Arch, the toolchain can be set up with the following command.

	sudo pacman -S avr-gcc avr-libc avrdude

Then, download the repository to your local machine

	git clone git@github.com:GGreenwood/Shinewave.git
    cd Shinewave
    git checkout v0.2-usb

There are two ways to install my program onto your ATtiny84: through the micronucleus bootloader, or directly through ISP. I recommend setting up the bootloader since it'll let you reprogram the board without needed to disassemble the controller.

Since the GameCube controller's signal line is connected to one of the ISP programming pins, you must disconnect the controller before programming. This is handled by removing the onboard jumper.

## Bootloader programming
Once you have the project downloaded, you'll need to do the following steps

* Assemble the board and connect your ISP programmer
* Update the `$(PROGRAMMER)` variable in `/bootloader/firmware/Makefile` to reflect your USB programmer
* Flash the micronucleus fuses
* Compile and burn the micronucleus bootloader
* Install the micronucleus command-line client
* Compile the Shinewave project and run the micronucleus uploader
* Connect your circuit to USB

The commands to do the above are as follows:

    # ./Shinewave
    cd bootloader/firmware/
    # update the PROGRAMMER setting
    vim Makefile
    # set the fuses and burn the bootloader
    make fuse
    make flash
    # set up the command-line programmer
    cd ../commandline/
    make
    sudo make install
    sudo cp 49-micronucleus.rules /etc/udev/rules.d/
    # Compile and flash Shinewave
    cd ../firmware/
    make microflash

## Direct programming
This approach is for flashing Shinewave to the board directly, as opposed to through a bootloader. Just like setting up the bootloader, you need an ISP programmer connected to the circuit. However, you'll need to use the ISP programmer every time you want to change the code, not just the first time

    # ./Shinewave
    cd firmware/
    make fuse
    make flash

## Dependencies
This project depends on several libraries, which are listed below.

* Josh Levine's simple [NeoPixel Demo](https://github.com/bigjosh/SimpleNeoPixelDemo) library
* [V-USB](https://www.obdev.at/products/vusb/index.html), for USB communications
* The [micronucleus](https://github.com/micronucleus/micronucleus) USB bootloader

Special thanks also go to [JustANull](https://github.com/JustANull) for C help and [Code and Life](http://codeandlife.com/) for having so many amazing V-USB tutorials.

# Schematics and Board
The schematics and layout for this project were done in EAGLE. The source files are located in the `/circuit` folder, and exported images of the output are available below, or in `/images`.

![Schematics](/images/schematic.png)
![Board](/images/board.png)

# Changelog

## [v0.2](https://github.com/GGreenwood/Shinewave/releases/tag/v0.2)
[Original post](http://electricexploits.net/gamecube-controller-usb-conversion-mod/)

* Added schematics and layout
* Added USB joystick functionality
* Added micronucleus bootloader
* Removed protothread dependency
* Changed from the ATtiny85 to the ATtiny84
* Reduced running voltage from 5V to 3.3V and clock speed from 16MHz to 12MHz

## [v0.1.1](https://github.com/GGreenwood/Shinewave/tree/v0.1.1)

* Added a variable delay to fix compatibility with nonstandard messaging speeds. 
* Fixed up Makefile and documentation.

## [v0.1](https://github.com/GGreenwood/Shinewave/releases/tag/v0.1-falco)
[Video](https://www.youtube.com/watch?v=1U4EOI_aFdc)

[Original post](http://electricexploits.net/shinewave/)

[Programming guide](http://electricexploits.net/programming-the-waveshine-prototype-part1/)

[Signal reading overview](http://electricexploits.net/programming-the-waveshine-prototype-part2/)

[Featured on Hackaday](http://hackaday.com/2015/08/02/shinewave-gamecube-controller-is-shiny/) 

[Featured on Kotaku](http://kotaku.com/gamecube-controller-responds-to-smash-bros-moves-with-1721711510)

* Initial release
* Reactive LED mod
* Hard-coded Falco animations, using the protothread library
