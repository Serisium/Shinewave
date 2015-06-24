PROJECTNAME = blinkcube
DEVICE = attiny85
PROGRAMMER = usbtiny
F_CPU = 16000000
CXX = avr-g++
AVROBJCOPY = avr-objcopy
FILES = main.c ANA_COMP.S

TARGET = $(PROJECTNAME).hex

CFLAGS = -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU) $(FILES)
OBJFLAGS = -O ihex -R .eeprom $(PROJECTNAME).obj $(TARGET)
AVRFLAGS = -p $(DEVICE) -c 


all:	$(TARGET)

flash:	$(TARGET)
	avrdude -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET)

clean:
	rm -f *.0 *.hex *.obj

blinkcube.hex: blinkcube.obj
	$(AVROBJCOPY) $(OBJFLAGS)

blinkcube.obj: $(FILES)
	$(CXX) -o $(PROJECTNAME).obj $(CFLAGS)
