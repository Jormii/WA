TARGET = WA_$(BUILD)

MAIN = main.o
OBJS = 	cb.o src/types.o src/vfpu.o src/wa.o src/wv_obj.o \
		C-CPP-CodeBase/c.o

MAIN_TEST = test.o
OBJS_TEST = tests/vfpu.o \
			C-CPP-CodeBase/tests/c.o C-CPP-CodeBase/tests/cpp.o

LIBS = -lstdc++ -lpspvfpu

INCDIR = inc C-CPP-CodeBase
CFLAGS = -Wall -Wextra -Wpedantic
CXXFLAGS = $(CFLAGS) -std=gnu++23 -fno-rtti -fexceptions
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

PRXS = 0 1
ifeq ($(PRX), 0)
BUILD_PRX = 0
CFLAGS := $(CFLAGS) -g3 -pg
LDFLAGS := $(LDFLAGS) -g3 -pg
else ifeq ($(PRX), 1)
BUILD_PRX = 1
else
$(error Unsupported PRX=$(PRX). Allowed: $(PRXS))
endif

BUILDS = TEST DEBUG RELEASE
ifeq ($(BUILD), TEST)
OBJS := $(MAIN_TEST) $(OBJS) $(OBJS_TEST)
CFLAGS := $(CFLAGS) -g3
else ifeq ($(BUILD), DEBUG)
OBJS := $(MAIN) $(OBJS)
CFLAGS := $(CFLAGS) -g3
else ifeq ($(BUILD), RELEASE)
OBJS := $(MAIN) $(OBJS)
CFLAGS := $(CFLAGS) -O2 -D NDEBUG
else
$(error Unsupported BUILD=$(BUILD). Allowed: $(BUILDS))
endif

PPSSPPS = 0 1
ifeq ($(PPSSPP), 0)
CFLAGS := $(CFLAGS)
else ifeq ($(PPSSPP), 1)
CFLAGS := $(CFLAGS) -D PPSSPP
else
$(error Unsupported PPSSPP=$(PPSSPP). Allowed: $(PPSSPPS))
endif

DEBUGGERS = 0 1
ifeq ($(DEBUGGER), 0)
CFLAGS := $(CFLAGS)
else ifeq ($(DEBUGGER), 1)
CFLAGS := $(CFLAGS) -g3 -D DEBUGGER
else
$(error Unsupported DEBUGGER=$(DEBUGGER). Allowed: $(DEBUGGERS))
endif

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = $(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
