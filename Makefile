PROJECTNAME = shinewave
DEVICE = attiny85
PROGRAMMER = usbtiny
F_CPU = 16000000
CXX = avr-g++
AVROBJCOPY = avr-objcopy
#FILES = main.c ANA_COMP.S
FILES = main.c

TARGET = $(PROJECTNAME).hex

CFLAGS = -Wall -O2 -mmcu=$(DEVICE) -DF_CPU=$(F_CPU) $(FILES)
OBJFLAGS = -O ihex -R .eeprom $(PROJECTNAME).obj $(TARGET)
AVRFLAGS = -p $(DEVICE) -c $(PROGRAMMER)

EFUSE = 0xFF
HFUSE = 0xDF
LFuse = 0xD1

all:	$(TARGET)

flash:	$(TARGET)
	avrdude $(AVRFLAGS) -U flash:w:$(TARGET)

clean:
	rm -f *.0 *.hex *.obj

fuse:
	avrdude $(AVRFLAGS) -U efuse:w:$(EFUSE):m -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

shinewave.hex: shinewave.obj
	$(AVROBJCOPY) $(OBJFLAGS)

shinewave.obj: $(FILES)
	$(CXX) -o $(PROJECTNAME).obj $(CFLAGS)
