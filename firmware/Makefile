PROJECTNAME = shinewave
DEVICE = attiny84
PROGRAMMER = usbtiny
F_CPU = 12000000
CC = avr-gcc
AVROBJCOPY = avr-objcopy

LFUSE = 0xEF
HFUSE = 0xDF
EFUSE = 0xFF

CFLAGS = -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU) -std=gnu99
OBJFLAGS = -j .text -j .data -O ihex
AVRFLAGS = -p $(DEVICE) -c $(PROGRAMMER)

COBJECTS = $(patsubst %.c, %.o, $(shell find $(SOURCEDIR) -name '*.c'))
ASMOBJECTS = $(patsubst %.S, %.o, $(shell find $(SOURCEDIR) -name '*.S'))
OBJECTS = $(COBJECTS) $(ASMOBJECTS)
HEADERS = $(wildcard *.h)

TARGET = $(PROJECTNAME).hex

all:	$(TARGET)

microflash:	$(TARGET)
	micronucleus --run $(PROJECTNAME).hex

flash:	$(TARGET)
	avrdude $(AVRFLAGS) -U flash:w:$(TARGET)

clean:
	rm -f *.0 *.hex *.o usbdrv/*.o

fuse:
	avrdude $(AVRFLAGS) -U efuse:w:$(EFUSE):m -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

%.hex: %.elf
	$(AVROBJCOPY) $(OBJFLAGS) $< $@

$(PROJECTNAME).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# From C source to .o object file
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# From assembler source to .o object file
%.o: %.S $(HEADERS)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@
