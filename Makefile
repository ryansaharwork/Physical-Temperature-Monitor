DEVICE     = atmega328p
CLOCK      = 16000000
PROGRAMMER = -c arduino -b 115200 -P COM3
OBJECTS    = main.o lcd.o ds18b20.o encoder.o timer.o serial.o
FUSES      = -U hfuse:w:0xde:m -U lfuse:w:0xff:m -U efuse:w:0x05:m

# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -std=c99 -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

# Dependencies
main.o: main.c project.h lcd.h ds18b20.h encoder.h timer.h serial.h
lcd.o: lcd.c lcd.h
ds18b20.o: ds18b20.c ds18b20.h
encoder.o: encoder.c encoder.h project.h
timer.o: timer.c timer.h project.h
serial.o: serial.c serial.h project.h

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c

test: test.hex
	$(AVRDUDE) -U flash:w:test.hex:i