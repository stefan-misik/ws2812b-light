# Makefile for embedded part of WS2812B Christmas lights

CC     = avr-gcc
CXX    = avr-g++
OBJCPY = avr-objcopy
OBJDMP = avr-objdump
SIZE   = avr-size
AD     = avrdude

# GCC MCU
MCU      = atmega328p
DEF_FREQ = 16000000
DEF_MCU  = AVR_ATmega328P
# Avrdude part name
PART = m328p
# Avrdude programmer name
PROG = usbasp
# Avrdude port name
PORT = usb
# Project name
PROJ = ws2812b-x-mass
# Source files
SRC  = system.cpp main.cpp led_controller.cpp animations/none.cpp         \
       animations/rainbow.cpp animations/drip.cpp

CPPFLAGS  = -I. -DF_CPU=$(DEF_FREQ)UL -D__$(DEF_MCU)__
CFLAGS    = -c -ggdb -Og -mmcu=$(MCU)
CXXFLAGS  = -c -ggdb -Og -std=c++14 -fno-exceptions -mmcu=$(MCU)
LDFLAGS   = -ggdb -Og -mmcu=$(MCU)
LDLIBS    =
SIZEFLAGS = -C --mcu=$(MCU)

################################################################################
OBJ = $(addsuffix .o, $(basename $(SRC)))
DEP = $(OBJ:.o=.d)

.PHONY: all clean stats prog

all: $(PROJ).elf $(PROJ).hex $(PROJ).lss stats

include $(DEP)

stats: $(PROJ).elf
	$(SIZE) $(SIZEFLAGS) $<

prog: $(PROJ).hex
	$(AD) -c $(PROG) -p $(PART) -P $(PORT) -U flash:w:$<:i

$(PROJ).elf: $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $@ $(LOADLIBES) $(LDLIBS)

$(PROJ).hex: $(PROJ).elf
	$(OBJCPY) -R .eeprom -O ihex $< $@

$(PROJ).lss: $(PROJ).elf
	$(OBJDMP) -h -S $<  > $@

%.d: %.c
	$(CC) -MM $(CPPFLAGS) $(CFLAGS) $< -MF $@

%.d: %.cpp
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< -MF $@

clean:
	$(RM) $(PROJ).elf $(PROJ).hex $(PROJ).lss $(OBJ) $(DEP)

