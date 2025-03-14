TARGET = WA_$(BUILD)

MAIN = main.o
OBJS = cb.o src/types.o src/wa.o

MAIN_TEST = test.o
OBJS_TEST = tests/vfpu.o

LIBS = -lstdc++ -lpspvfpu

INCDIR = inc C-CPP-CodeBase
CFLAGS = -Wall -Wextra -Wpedantic
CXXFLAGS = $(CFLAGS) -std=gnu++23 -fno-rtti -fexceptions
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

BUILDS = TEST DEBUG RELEASE
ifeq ($(BUILD), TEST)
OBJS := $(MAIN_TEST) $(OBJS) $(OBJS_TEST)
else ifeq ($(BUILD), DEBUG)
OBJS := $(MAIN) $(OBJS) 
else ifeq ($(BUILD), RELEASE)
OBJS := $(MAIN) $(OBJS)
CFLAGS := -D NDEBUG
else
$(error Unsupported BUILD=$(BUILD). Allowed: $(BUILDS))
endif

BUILD_PRX = 1
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = $(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
