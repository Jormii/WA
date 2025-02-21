TARGET = WA
OBJS = main.o src/wa.o

LIBS = -lstdc++ -lpspvfpu

INCDIR = inc C-CPP-CodeBase
CFLAGS = -Wall -Wextra -Wpedantic
CXXFLAGS = $(CFLAGS) -fno-rtti -fexceptions -std=gnu++23
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

BUILD_PRX = 1
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = WA

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
