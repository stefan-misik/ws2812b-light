PROJ = tests

# Sources
SRC =  \
    test.cpp

BUILDDIR = _build
OPTFLAGS = -Og
DEBUGDEFINE = DEBUG

DEFINE    = $(DEBUGDEFINE)
INCLUDE   = ../fw/stm32g0 ../fw/shared
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

.PHONY: all clean

all: $(OUT)

include ../fw/rules.mk

clean:
	$(RM) -r _build/
