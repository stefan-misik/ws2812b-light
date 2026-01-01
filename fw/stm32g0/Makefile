# Makefile for embedded part of WS2812B Christmas lights for STM32G0 processors


TOOLCHAIN_PREFIX = arm-none-eabi

# Project name
PROJ = ws2812b-light
# Source files
SRC  =  \
    $(addprefix support/,  \
        startup_stm32g070kbtx.s  \
        sysmem.c  \
        syscalls.c  \
        system_stm32g0xx.c  \
    )  \
    $(addprefix tools/,  \
        button_filter.cpp  \
    )  \
    $(addprefix driver/,  \
        base.cpp  \
        common.cpp  \
        systick.cpp  \
        led_controller.cpp  \
        ir_receiver.cpp  \
        buzzer.cpp  \
        keypad.cpp  \
        status_leds.cpp  \
        cpu_usage.cpp  \
        i2c_bus.cpp  \
        $(addprefix i2c_bus/,  \
            cat24cx.cpp  \
        )  \
    )  \
    $(addprefix app/,  \
        $(addprefix tools/,  \
            color.cpp  \
        )  \
        $(addprefix input/,  \
            keypad.cpp  \
            ir_remote.cpp  \
        )  \
        $(addprefix animation/,  \
            color.cpp  \
            rainbow.cpp  \
            retro.cpp  \
            twinkle.cpp  \
            shifting_color.cpp  \
            lights.cpp  \
        )  \
        music.cpp  \
        animation_storage.cpp  \
        led_correction.cpp  \
        event_queue.cpp  \
        input.cpp  \
        io.cpp  \
        lights.cpp  \
        main.cpp  \
    )

ifeq ($(strip $(DBG)),yes)
BUILDDIR = _build/debug
OPTFLAGS = -Og
DEBUGDEFINE = DEBUG
else
BUILDDIR = _build/release
OPTFLAGS = -Og
DEBUGDEFINE = NDEBUG
endif

DEFINE    = $(MCU_DEFINE) $(DEBUGDEFINE)
INCLUDE   = . ../shared
CPPFLAGS  =
CFLAGS    = -g3 $(OPTFLAGS) -Wall -Wextra -Werror
CXXFLAGS  = -g3 $(OPTFLAGS) -Wall -Wextra -Werror
LDFLAGS   = -g3 $(OPTFLAGS)
LDLIBS    = -specs=nano.specs -lc -lm

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

# Use simple wear leveling, by putting debug binary in second half of the flash memory
ifeq ($(strip $(DBG)),yes)
LDFLAGS += -Wl,--defsym=_Fw_Offset=64K
endif

################################################################################

.PHONY: all clean songs

all: $(OUT_HEX)

# Make output binary depend on linker script
$(OUT): $(LDSCR)

include ../rules.mk

$(OUT_HEX): $(OUT)
	$(OCP) --strip-all $< -O ihex $@

app/music.cpp: songs
songs:
	$(MAKE) -C ../shared/songs all

clean:
	$(RM) -r _build
	$(MAKE) -C ../shared/songs clean
