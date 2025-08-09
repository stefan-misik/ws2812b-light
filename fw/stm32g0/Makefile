# Makefile for embedded part of WS2812B Christmas lights for STM32G0 processors


TOOLCHAIN_PREFIX = arm-none-eabi

# Project name
PROJ = ws2812b-light
# Source files
SRC  =  \
    support/startup_stm32g070kbtx.s  \
    support/sysmem.c  \
    support/syscalls.c  \
    support/system_stm32g0xx.c  \
    tools/button_filter.cpp  \
    driver/base.cpp  \
    driver/systick.cpp  \
    driver/led_controller.cpp  \
    driver/ir_receiver.cpp  \
    driver/buzzer.cpp  \
    driver/keypad.cpp  \
    driver/status_leds.cpp  \
    input/keypad.cpp  \
    input/ir_remote.cpp  \
    led_correction.cpp  \
    event_queue.cpp  \
    input.cpp  \
    lights.cpp  \
    main.cpp

ifeq ($(strip $(DBG)),yes)
BUILDDIR = Debug
OPTFLAGS = -Og
else
BUILDDIR = Release
OPTFLAGS = -Og
endif

DEFINE    = $(MCU_DEFINE)
INCLUDE   = . ../shared
CPPFLAGS  =
CFLAGS    = -g3 $(OPTFLAGS) -Wall -Wextra
CXXFLAGS  = -g3 $(OPTFLAGS) -Wall -Wextra
LDFLAGS   = -g3 $(OPTFLAGS) -Wl,-Map=$(PROJ).map
LDLIBS    =

# C specific
CFLAGS += -std=c99
# C++ specific
CXXFLAGS += -std=c++20 -fno-exceptions -fno-rtti
# Suppress unwanted warnings
CXXFLAGS += -Wno-register -Wno-volatile

# Common
COMMON_FLAGS = -ffunction-sections -fdata-sections
CXXFLAGS += $(COMMON_FLAGS)
CFLAGS += $(COMMON_FLAGS)

# Linker specific
LDFLAGS += -Wl,--gc-sections -Wl,-Map=$(BUILDDIR)/$(PROJ).map -Wl,--cref

# Vendor CMSIS
VENDOR_STM = vendor/STM32CubeG0
VENDOR_STM_CMSIS = $(VENDOR_STM)/Drivers/CMSIS
LDSCR = STM32G070KBTX_FLASH.ld
DEFINE += STM32 STM32G0 STM32G070KBTx
INCLUDE +=  \
    $(VENDOR_STM_CMSIS)/Device/ST/STM32G0xx/Include  \
    $(VENDOR_STM_CMSIS)/Include
VENDOR_STM_FLAGS = -mthumb -mcpu=cortex-m0plus
CFLAGS += $(VENDOR_STM_FLAGS)
CXXFLAGS += $(VENDOR_STM_FLAGS)
LDFLAGS += $(VENDOR_STM_FLAGS) $(addprefix -T,$(LDSCR))

# Vendor STM32 Drivers
VENDOR_STM_DRIVER = $(VENDOR_STM)/Drivers/STM32G0xx_HAL_Driver
DEFINE += STM32G070xx
INCLUDE += $(VENDOR_STM_DRIVER)/Inc
VENDOR_STM_DRIVER_LL = rcc utils pwr gpio dma iwdg tim i2c usart
SRC += $(wildcard $(addsuffix .c,$(addprefix $(VENDOR_STM_DRIVER)/Src/stm32g0xx_ll_,  \
    $(VENDOR_STM_DRIVER_LL)  \
)))
DEFINE += USE_FULL_LL_DRIVER

OUT = $(BUILDDIR)/$(PROJ).elf
OUT_HEX = $(OUT:.elf=.hex)

################################################################################

.PHONY: all clean

all: $(OUT) $(OUT_HEX)

include ../rules.mk

#$(error $(OUT_HEX))
$(OUT_HEX): $(OUT)
	$(OCP) --strip-all $< -O ihex $@

music.cpp: $(addprefix songs/,$(addsuffix .inc,$(SONGS)))

clean:
	$(RM) -r Debug/ Release/
