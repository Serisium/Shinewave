PROJECTNAME = blinkcube
DEVICE = attiny85
PROGRAMMER = usbtiny
F_CPU = 16000000
CXX = avr-g++
AVROBJCOPY = avr-objcopy
FILES = main.c INT0.S

TARGET = $(PROJECTNAME).hex

CFLAGS = -Wall -Os -mmcu=$(DEVICE) -DF_CPU=$(F_CPU) $(FILES)
OBJFLAGS = -O ihex -R .eeprom $(PROJECTNAME).obj $(TARGET)
AVRFLAGS = -p $(DEVICE) -c 



#avr-g++ -mmcu=attiny85 -DF_CPU=16000000 -Wall -Os main.c INT0.S 
#avr-objcopy -O ihex -R .eeprom a.out main.hex
#avrdude -p t85 -c usbtiny -U flash:w:main.hex 

all:	$(TARGET)

flash:	$(TARGET)
	avrdude -p $(DEVICE) -c $(PROGRAMMER) -U flash:w:$(TARGET)

clean:
	rm -f *.0 *.hex *.obj

blinkcube.hex: blinkcube.obj
	$(AVROBJCOPY) $(OBJFLAGS)

blinkcube.obj: $(FILES)
	$(CXX) -o $(PROJECTNAME).obj $(CFLAGS)
