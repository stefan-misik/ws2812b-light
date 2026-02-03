PROJ = tests

define test_source
$1 $(wildcard ../stm32g0/$1)
endef

# Sources
SRC =  \
    $(call test_source,app/storage/block_counter.cpp)

BUILDDIR = _build
OPTFLAGS = -Og
DEBUGDEFINE = DEBUG

DEFINE    = $(DEBUGDEFINE)
INCLUDE   = ../stm32g0 ../shared
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

include ../rules.mk

clean:
	$(RM) -r _build/
