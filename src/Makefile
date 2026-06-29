REPO_ROOT := ..
OUT := $(REPO_ROOT)

XDIR ?= /u/cs452/public/xdev
ARCH := cortex-a72
TRIPLE := aarch64-none-elf
XBINDIR := $(XDIR)/bin
CC := $(XBINDIR)/$(TRIPLE)-gcc
OBJCOPY := $(XBINDIR)/$(TRIPLE)-objcopy
OBJDUMP := $(XBINDIR)/$(TRIPLE)-objdump

MODE ?= hw
ifeq ($(MODE),qemu)
    MODE_CFLAGS := -DMARKLIN_HW_UART3=0 -DBUILD_QEMU=1
else ifeq ($(MODE),hw)
    MODE_CFLAGS := -DMARKLIN_HW_UART3=1 -DBUILD_HW=1
else
    $(error MODE must be 'hw' or 'qemu' (got '$(MODE)'))
endif

WARNINGS := -Wall -Wextra -Wpedantic -Wno-unused-const-variable -Wno-incompatible-pointer-types -Wno-return-mismatch -Wno-int-conversion -Wno-implicit-function-declaration
SERVER_INCLUDES := $(addprefix -I,$(sort $(dir $(wildcard \
	k2/servers/*/*.h k3/servers/*/*.h k4/servers/*/*.h))))
INC := -Ik0 -Ik1 -Ik2 -Ik3 -Ik4 -Icommon -Icommon/custom -Itc1 -Itests \
	$(SERVER_INCLUDES)

CFLAGS := -g -pipe -static $(WARNINGS) -ffreestanding -nostartfiles \
	-mcpu=$(ARCH) -static-pie -mstrict-align -fno-builtin -mgeneral-regs-only \
	$(INC) $(MODE_CFLAGS)

LDFLAGS := -Wl,-nmagic -Wl,-Tk0/linker.ld
LINKER := k0/linker.ld

K2_SERVER_SOURCES := $(wildcard k2/servers/*/*.c)
K3_SERVER_SOURCES := $(wildcard k3/servers/*/*.c)
K4_SERVER_SOURCES := $(wildcard k4/servers/*/*.c)

SOURCES := $(wildcard tc1/*.c) $(wildcard tests/*.c) \
	$(wildcard k1/*.c) $(wildcard k0/*.S) \
	$(wildcard k2/*.c) $(wildcard k3/*.c) $(wildcard k4/*.c) \
	common/kmain.c common/rpi.c common/util.c \
	$(K2_SERVER_SOURCES) $(K3_SERVER_SOURCES) $(K4_SERVER_SOURCES) \
	$(wildcard common/custom/*.c)

OBJECTS := $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(SOURCES)))
DEPENDS := $(patsubst %.c,%.d,$(patsubst %.S,%.d,$(SOURCES)))

KERNEL_ELF := $(OUT)/0-d273liu.elf
KERNEL_IMG := $(OUT)/0-d273liu.img

.PHONY: all clean hw qemu 0-d273liu.img 0-d273liu.elf

all: 0-d273liu.img

hw:
	$(MAKE) MODE=hw all

qemu:
	$(MAKE) MODE=qemu all

0-d273liu.img: $(KERNEL_IMG)
0-d273liu.elf: $(KERNEL_ELF)

clean:
	find . \( -name '*.o' -o -name '*.d' \) -delete
	rm -f $(KERNEL_ELF) $(KERNEL_IMG)

$(KERNEL_IMG): $(KERNEL_ELF)
	$(OBJCOPY) $< -O binary $@

$(KERNEL_ELF): $(OBJECTS) $(LINKER)
	$(CC) $(CFLAGS) $(filter-out %.ld,$^) -o $@ $(LDFLAGS)
	@$(OBJDUMP) -d $@ | fgrep -q q0 && printf "\n***** WARNING: SIMD INSTRUCTIONS DETECTED! *****\n\n" || true

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

%.o: %.S Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDS)
