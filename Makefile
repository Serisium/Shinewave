avr-g++ -mmcu=attiny85 -DF_CPU=16000000 -Wall -Os main.c INT0.S 
avr-objcopy -O ihex -R .eeprom a.out main.hex
avrdude -p t85 -c usbtiny -U flash:w:main.hex 
