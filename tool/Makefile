PYTHON ?= python
PYBIND11_INCLUDES := $(shell $(PYTHON) -m pybind11 --includes)
EXT_SUFFIX := $(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX'))")
PYTHON_VER := $(shell $(PYTHON) -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")

PROJ = animations
ORIG_PROJ = ../fw/stm32g0

# Sources
SRC =  \
    $(ORIG_PROJ)/app/tools/color.cpp  \
    $(ORIG_PROJ)/app/animation_storage.cpp  \
    $(ORIG_PROJ)/app/animation/tools/color_themes.cpp  \
    $(wildcard $(ORIG_PROJ)/app/animation/*.cpp)  \
    animations.cpp

ifeq ($(strip $(DBG)),yes)
BUILDDIR = _build/debug
OPTFLAGS = -Og
DEBUGDEFINE = DEBUG
else
BUILDDIR = _build/release
OPTFLAGS = -Og
DEBUGDEFINE = NDEBUG
endif

DEFINE    = STM32 $(DEBUGDEFINE)
INCLUDE   = $(ORIG_PROJ) $(ORIG_PROJ)/../shared
CPPFLAGS  = $(PYBIND11_INCLUDES)
CFLAGS    = -g3 $(OPTFLAGS) -Wall -Wextra -Werror
CXXFLAGS  = -g3 $(OPTFLAGS) -Wall -Wextra -Werror
LDFLAGS   = -g3 $(OPTFLAGS) -shared
LDLIBS    = -lpython$(PYTHON_VER)

# C specific
CFLAGS += -std=c99
# C++ specific
CXXFLAGS += -std=c++20
# Suppress unwanted warnings
CXXFLAGS += -Wno-register -Wno-volatile

# Common
COMMON_FLAGS = -fPIC
CXXFLAGS += $(COMMON_FLAGS)
CFLAGS += $(COMMON_FLAGS)

OUT = $(BUILDDIR)/$(PROJ)$(EXT_SUFFIX)
STUBS = $(PROJ).pyi
INSTALL = $(PROJ)$(EXT_SUFFIX)

################################################################################

.PHONY: all clean install stubs

all: $(OUT)
	cp $< $(INSTALL)

include ../fw/rules.mk

clean:
	$(RM) -r _build/ $(INSTALL)

stubs: $(STUBS)

$(STUBS): $(PROJ).cpp
	PYTHONPATH=. $(PYTHON) -m pybind11_stubgen -o . $(PROJ)
	dos2unix $@
