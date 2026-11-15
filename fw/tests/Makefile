PROJ = tests

# Sources
SRC =  \
    test.cpp  \
    cat24cx_tests.cpp  \
    crc_tests.cpp  \
    records_tests.cpp  \
    ../stm32g0/app/records.cpp  \
    ../stm32g0/app/records/crc.cpp  \
    ../stm32g0/driver/i2c_bus/cat24cx.cpp

BUILDDIR = _build
OPTFLAGS = -Og
DEBUGDEFINE = DEBUG

DEFINE    = $(DEBUGDEFINE)
INCLUDE   = host_include ../stm32g0 ../shared
CPPFLAGS  =
CFLAGS    = -g3 $(OPTFLAGS) -Wall -Wextra -Werror
CXXFLAGS  = -g3 $(OPTFLAGS) -Wall -Wextra -Werror
LDFLAGS   = -g3 $(OPTFLAGS)
LDLIBS    = -lCatch2Main -lCatch2

# C specific
CFLAGS += -std=c99
# C++ specific
CXXFLAGS += -std=c++20
# Suppress unwanted warnings
CXXFLAGS += -Wno-register -Wno-volatile

# Common
COMMON_FLAGS =
CXXFLAGS += $(COMMON_FLAGS)
CFLAGS += $(COMMON_FLAGS)

OUT = $(BUILDDIR)/$(PROJ).exe

################################################################################

.PHONY: all test clean

all: $(OUT)

test: $(OUT)
	./$(OUT)

include ../rules.mk

clean:
	$(RM) -r _build/
