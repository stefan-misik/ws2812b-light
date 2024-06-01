# Makefile for embedded part of WS2812B Christmas lights

LED_ORDER = red,green,blue

CC     = avr-gcc
CXX    = avr-g++
OBJCPY = avr-objcopy
OBJDMP = avr-objdump
SIZE   = avr-size
AD     = avrdude

# GCC MCU
MCU      = attiny85
DEF_FREQ = 16000000
DEF_MCU  = AVR_ATtiny85
# Avrdude part name
PART = t85
# Avrdude programmer name
PROG = usbasp
# Avrdude port name
PORT = usb
# Project name
PROJ = ws2812b-light
# Source files
SRC  =  \
    analog_in.cpp  \
    system.cpp  \
    main.cpp  \
    led_controller.cpp  \
    music.cpp  \
    status_led.cpp  \
    nvm_storage.cpp  \
    time_service.cpp buttons.cpp  \
    tools/button_filter.cpp  \
    tools/color.cpp  \
    tools/id_selector.cpp  \
    $(addprefix animations/,  \
        color.cpp  \
        rainbow.cpp  \
        retro.cpp  \
        twinkle.cpp  \
        shifting_color.cpp  \
    )  \
    animation_list.cpp

SONGS =  \
    silent_night  \
    jingle_bells

CPPFLAGS  = -I. -DF_CPU=$(DEF_FREQ)UL -D__$(DEF_MCU)__ -DLED_ORDER=$(LED_ORDER)
CFLAGS    = -c -ggdb -Os -Wall -Wno-array-bounds -mmcu=$(MCU)
CXXFLAGS  = -c -ggdb -Os -Wall -Wno-array-bounds -mmcu=$(MCU) -std=c++14 -fno-exceptions
LDFLAGS   = -ggdb -Os -mmcu=$(MCU) -Wl,-Map=$(PROJ).map
LDLIBS    =
SIZEFLAGS = -C --mcu=$(MCU)

################################################################################
OBJ = $(addsuffix .o, $(basename $(SRC)))
DEP = $(OBJ:.o=.d)

.PHONY: all clean stats prog fuses

all: $(PROJ).elf $(PROJ).hex $(PROJ).lss stats

-include $(DEP)

music.cpp: $(addprefix songs/,$(addsuffix .inc,$(SONGS)))

stats: $(PROJ).elf
	$(SIZE) $(SIZEFLAGS) $<

prog: $(PROJ).hex
	$(AD) -c $(PROG) -p $(PART) -P $(PORT) -U flash:w:$<:i

fuses:
	$(AD) -c $(PROG) -p $(PART) -P $(PORT) -B 1000 -D           \
	    -U lfuse:w:0xF1:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m

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
	$(RM) $(PROJ).elf $(PROJ).hex $(PROJ).lss $(OBJ) $(DEP) songs/*.inc

%.inc: %.tune
	python songs/compose.py $< -o $@
