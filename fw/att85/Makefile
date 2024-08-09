# Makefile for embedded part of WS2812B Christmas lights

LED_ORDER = red,green,blue

TOOLCHAIN_PREFIX = avr
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
    time_service.cpp  \
    buttons.cpp  \
    ir_receiver.cpp  \
    ../shared/tools/button_filter.cpp  \
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

DEFINE    = F_CPU=$(DEF_FREQ)UL __$(DEF_MCU)__ LED_ORDER=$(LED_ORDER)
INCLUDE   = . ../shared
CPPFLAGS  =
CFLAGS    = -ggdb -Os -Wall -Wno-array-bounds -mmcu=$(MCU)
CXXFLAGS  = -ggdb -Os -Wall -Wno-array-bounds -mmcu=$(MCU) -std=c++14 -fno-exceptions
LDFLAGS   = -ggdb -Os -mmcu=$(MCU) -Wl,-Map=$(PROJ).map
LDLIBS    =
SIZEFLAGS = -C --mcu=$(MCU)
BUILDDIR = build
OUT = $(BUILDDIR)/$(PROJ).elf
OUT_HEX = $(OUT:.elf=.hex)
OUT_LSS = $(OUT:.elf=.lss)

################################################################################

.PHONY: all clean stats prog fuses

all: $(OUT) $(OUT_HEX) $(OUT_LSS) stats

include ../rules.mk

music.cpp: $(addprefix songs/,$(addsuffix .inc,$(SONGS)))

stats: $(OUT)
	$(SZ) $(SIZEFLAGS) $<

prog: $(OUT_HEX)
	$(AD) -c $(PROG) -p $(PART) -P $(PORT) -U flash:w:$<:i

fuses:
	$(AD) -c $(PROG) -p $(PART) -P $(PORT) -B 1000 -D           \
	    -U lfuse:w:0xF1:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m

$(OUT_HEX): $(OUT)
	$(OCP) -R .eeprom -O ihex $< $@

$(OUT_LSS): $(OUT)
	$(ODP) -h -S $<  > $@

clean:
	$(RM) $(CLEANUP) $(OUT_HEX) $(OUT_LSS) songs/*.inc

%.inc: %.tune
	python songs/compose.py $< -o $@
