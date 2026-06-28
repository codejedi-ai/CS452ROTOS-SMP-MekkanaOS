# Honour XDIR from the environment (Docker sets it to /opt/xdev). Fall back
# to the original developer path so existing local builds keep working.
XDIR ?= /Users/darcyliu/bin/xdev
ARCH=cortex-a72
TRIPLE=aarch64-none-elf
XBINDIR:=$(XDIR)/bin
CC:=$(XBINDIR)/$(TRIPLE)-gcc
OBJCOPY:=$(XBINDIR)/$(TRIPLE)-objcopy
OBJDUMP:=$(XBINDIR)/$(TRIPLE)-objdump

# MODE selects the Marklin transport / runtime target.
#   MODE=hw   (default) - PL011 UART3 @ 0xFE201600, 2400 baud. Real Pi 4 hat.
#   MODE=qemu           - BCM2835 AUX mini-UART @ 0xFE215000. QEMU raspi4b.
# The ttyd web demo always builds with MODE=qemu.
MODE ?= hw
ifeq ($(MODE),qemu)
    MODE_CFLAGS := -DMARKLIN_HW_UART3=0 -DBUILD_QEMU=1
else ifeq ($(MODE),hw)
    MODE_CFLAGS := -DMARKLIN_HW_UART3=1 -DBUILD_HW=1
else
    $(error MODE must be 'hw' or 'qemu' (got '$(MODE)'))
endif

# COMPILE OPTIONS
# -ffunction-sections causes each function to be in a separate section (linker script relies on this)
WARNINGS=-Wall -Wextra -Wpedantic -Wno-unused-const-variable

CFLAGS:=-g -pipe -static $(WARNINGS) -ffreestanding -nostartfiles\
	-mcpu=$(ARCH) -static-pie -mstrict-align -fno-builtin -mgeneral-regs-only \
	-Isrc -Isrc/k1 -Isrc/k2 -Isrc/k3 -Isrc/k4 \
	$(MODE_CFLAGS)

# -Wl,option tells g++ to pass 'option' to the linker with commas replaced by spaces
# doing this rather than calling the linker ourselves simplifies the compilation procedure
LDFLAGS:=-Wl,-nmagic -Wl,-Tsrc/linker.ld

# Source files and include dirs.
# All code lives under src/. src/k1..k4 are the kernel modules; src/ui, src/tc1,
# src/tests are higher-level subsystems; src/main.c is the entry point.

SOURCES := $(wildcard src/ui/*.c) $(wildcard src/tc1/*.c) $(wildcard src/tests/*.c) \
           $(wildcard src/k1/*.c) $(wildcard src/k1/*.S) \
           $(wildcard src/k2/*.c) $(wildcard src/k2/*.S) \
           $(wildcard src/k3/*.c) $(wildcard src/k3/*.S) \
           $(wildcard src/k4/*.c) $(wildcard src/k4/*.S) \
           $(wildcard src/*.c) $(wildcard src/*.S)
# Create .o and .d files for every .cc and .S (hand-written assembly) file
OBJECTS := $(patsubst %.c, %.o, $(patsubst %.S, %.o, $(SOURCES)))
DEPENDS := $(patsubst %.c, %.d, $(patsubst %.S, %.d, $(SOURCES)))

# The first rule is the default, ie. "make", "make all" and "make 0-d273liu8.img" mean the same
all: 0-d273liu.img

clean:
	rm -f $(OBJECTS) $(DEPENDS) 0-d273liu.elf 0-d273liu.img

0-d273liu.img: 0-d273liu.elf
	$(OBJCOPY) $< -O binary $@

0-d273liu.elf: $(OBJECTS) src/linker.ld
	$(CC) $(CFLAGS) $(filter-out %.ld, $^) -o $@ $(LDFLAGS)
	@$(OBJDUMP) -d 0-d273liu.elf | fgrep -q q0 && printf "\n***** WARNING: SIMD INSTRUCTIONS DETECTED! *****\n\n" || true

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

%.o: %.S Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Convenience: two named modes -> two image files side-by-side.
hw:
	$(MAKE) MODE=hw

qemu:
	$(MAKE) MODE=qemu

-include $(DEPENDS)
