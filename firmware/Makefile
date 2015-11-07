PROJECTNAME = shinewave
DEVICE = attiny84
PROGRAMMER = usbtiny
F_CPU = 12000000
CXX = avr-gcc
AVROBJCOPY = avr-objcopy
FILES = main.c

TARGET = $(PROJECTNAME).hex

CFLAGS = -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU) -Iusbdrv -std=gnu99
#OBJFLAGS = -O ihex -R .eeprom $(PROJECTNAME).obj $(TARGET)
OBJFLAGS = -j .text -j .data -O ihex
AVRFLAGS = -p $(DEVICE) -c $(PROGRAMMER)

FUSE = 0xFF
HFUSE = 0xDF
LFUSE = 0xE0

OBJECTS = usbdrv/usbdrv.o usbdrv/oddebug.o usbdrv/usbdrvasm.o main.o

all:	$(TARGET)

flash:	$(TARGET)
	avrdude $(AVRFLAGS) -U flash:w:$(TARGET)

clean:
	rm -f *.0 *.hex *.o usbdrv/*.o

fuse:
	avrdude $(AVRFLAGS) -U efuse:w:$(EFUSE):m -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

%.hex: %.elf
	$(AVROBJCOPY) $(OBJFLAGS) $< $@

$(PROJECTNAME).elf: $(OBJECTS)
	$(CXX) $(CFLAGS) $(OBJECTS) -o $@

$(OBJECTS):	usbdrv/usbconfig.h

# C source to .o objects
%.o: %.c
	$(CXX) $(CFLAGS) -c $< -o $@

# ASM source to .o objects
%.o: %.S
	$(CXX) $(CFLAGS) -x assembler-with-cpp -c $< -o $@
