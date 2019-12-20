#!/usr/bin/make -f
# Makefile for embedded part of WS2812B Christmas lights

CC     = avr-gcc
CXX    = avr-g++
OBJCPY = avr-objcopy
OBJDMP = avr-objdump
SIZE   = avr-size
AD     = avrdude

# GCC MCU
MCU   = atmega328p
F_CPU = 16000000
# Avrdude part name
PART = m328p
# Avrdude programmer name
PROG = usbasp
# Avrdude port name
PORT = usb
# Project name
PROJ = ws2812b-x-mass
# Source files
SRC  = main.cpp

CPPFLAGS  = -DF_CPU=$(F_CPU)UL
CFLAGS    = -c -ggdb -Og -mmcu=$(MCU)
CXXFLAGS  = -c -ggdb -Og -mmcu=$(MCU)
LDFLAGS   = -ggdb -Og -mmcu=$(MCU)
SIZEFLAGS = -C --mcu=$(MCU)

#############################################
OBJ = $(addsuffix .o, $(basename $(SRC)))

.PHONY: all clean stats prog

all: $(PROJ).elf $(PROJ).hex $(PROJ).lss $(SRC)

stats: $(PROJ).elf
	$(SIZE) $(SIZEFLAGS) $<

prog: $(PROJ).hex
	$(AD) -c $(PROG) -p $(PART) -P $(PORT) -U flash:w:$<:i

$(PROJ).elf: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

$(PROJ).hex: $(PROJ).elf
	$(OBJCPY) -R .eeprom -O ihex $< $@

$(PROJ).lss: $(PROJ).elf
	$(OBJDMP) -h -S $<  > $@


clean:
	$(RM) $(PROJ).elf $(PROJ).hex $(PROJ).lss $(OBJ)

