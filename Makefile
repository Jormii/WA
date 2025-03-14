TARGET = WA_$(BUILD)
OBJS = cb.o src/types.o src/wa.o

BUILDS = TEST DEBUG
ifeq ($(BUILD), DEBUG)
OBJS := main.o $(OBJS) 
else ifeq ($(BUILD), TEST)
OBJS := test.o $(OBJS) tests/vfpu.o
else
$(error Unsupported BUILD=$(BUILD). Allowed: $(BUILDS))
endif


LIBS = -lstdc++ -lpspvfpu

INCDIR = inc C-CPP-CodeBase
CFLAGS = -Wall -Wextra -Wpedantic
CXXFLAGS = $(CFLAGS) -std=gnu++23 -fno-rtti -fexceptions
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

BUILD_PRX = 1
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = WA_$(BUILD)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
