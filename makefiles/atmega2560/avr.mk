CC=avr-gcc
AS=avr-gcc
INCLUDE_DIRS=-I. -I../avr_common
CC_OPTS=-Wall --std=gnu99 -DF_CPU=16000000UL -Os -funsigned-char -funsigned-bitfields  -fshort-enums -Wall -Wstrict-prototypes -mmcu=atmega2560 $(INCLUDE_DIRS)  -D__AVR_3_BYTE_PC__
AS_OPTS=-x assembler-with-cpp $(CC_OPTS)

AVRDUDE=avrdude
AVRDUDE_PORT = /dev/ttyACM0    

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET):i
AVRDUDE_FLAGS = -p m2560 -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b 115200
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)
AVRDUDE_FLAGS += -D -q -V -C /usr/share/arduino/hardware/tools/avr/../avrdude.conf
AVRDUDE_FLAGS += -c wiring

.phony:	clean all

all:	$(BINS) 

%.o:	%.c 
	$(CC) $(CC_OPTS) -c  -o $@ $<

%.o:	%.s 
	$(AS) $(AS_OPTS) -c  -o $@ $<

%.elf:	%.o $(OBJS)
	$(CC) $(CC_OPTS) -o $@ $< $(OBJS) $(LIBS)


%.hex:	%.elf
	avr-objcopy -O ihex -R .eeprom $< $@
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$@:i #$(AVRDUDE_WRITE_EEPROM) 

clean:	
	rm -rf $(OBJS) $(BINS) *.hex *~ *.o *.elf

.SECONDARY:	$(OBJS)
