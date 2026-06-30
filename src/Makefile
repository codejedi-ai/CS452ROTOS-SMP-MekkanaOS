# DarcyOS — single build definition (run via `make` at repo root or `make -C src` here).
REPO_ROOT := ..
OUT := $(REPO_ROOT)

XDIR ?= /u/cs452/public/xdev
ARCH := cortex-a72
TRIPLE := aarch64-none-elf
XBINDIR := $(XDIR)/bin
CC := $(XBINDIR)/$(TRIPLE)-gcc
OBJCOPY := $(XBINDIR)/$(TRIPLE)-objcopy
OBJDUMP := $(XBINDIR)/$(TRIPLE)-objdump

WARNINGS := -Wall -Wextra -Wpedantic -Wno-unused-const-variable -Wno-incompatible-pointer-types -Wno-int-conversion
SERVER_INCLUDES := $(addprefix -I,$(sort $(dir $(wildcard \
	k2/servers/*/*.h k3/servers/*/*.h k4/servers/*/*.h tc1/servers/*/*.h))))
INC := -Ik0 -Icommon/custom -Icommon/kernel -Icommon/smp -Ik1 -Ik1/test -Ik3 -Ik3/test -Ik4 -Ik4/test -Itc1 \
	-Ik2 -Ik2/test $(SERVER_INCLUDES) -Icommon -Idata -Idata/track_data -I$(REPO_ROOT)/src/tests

CFLAGS := -g -pipe -static $(WARNINGS) -ffreestanding -nostartfiles \
	-mcpu=$(ARCH) -static-pie -mstrict-align -fno-builtin -mgeneral-regs-only $(INC) \
	-DDEBUG=0 -DDEBUG_EXIT=0

LDFLAGS := -Wl,-nmagic -Wl,-Tk0/linker.ld
LINKER := k0/linker.ld

K3_DRIVER_SOURCES := k3/systimer.c
K2_SERVER_SOURCES := $(wildcard k2/servers/*/*.c)
K3_SERVER_SOURCES := $(wildcard k3/servers/*/*.c)
K4_SERVER_SOURCES := $(wildcard k4/servers/*/*.c)
TC1_SERVER_SOURCES := $(wildcard tc1/servers/*/*.c)
ALL_SERVER_SOURCES := $(K2_SERVER_SOURCES) $(K3_SERVER_SOURCES) $(K4_SERVER_SOURCES) $(TC1_SERVER_SOURCES)
COMMON_HW_SOURCES := common/rpi.c common/gic.c
SMP_SOURCES := $(wildcard common/smp/*.c)
DATA_SOURCES := data/track_data/track_data.c data/asciiart.c
COMMON_CUSTOM_SOURCES := common/custom/custmath.c common/custom/custstr.c
KERNEL_STUB_SOURCES := common/kernel/kernel_state.c common/kernel/sched_stubs.c common/kernel/ipc_stubs.c common/kernel/kernel_api.c common/kernel/ipc_dispatch.c
K1_TEST_SUITE := k1/test/k1tests.c
K2_TEST_SUITE := k2/test/k2tests.c
K3_TEST_SUITE := k3/test/k3tests_suite.c
K4_TEST_SUITE := k4/test/k4_smp_tests.c k4/test/k4tests.c

K1_SOURCES := $(filter-out k1/k1.c,$(wildcard k1/*.c))

SOURCES := $(wildcard tc1/*.c) \
	$(REPO_ROOT)/src/tests/boot_tests.c \
	$(REPO_ROOT)/src/tests/tc1tests.c \
	k1/k1.c k1/int64voodoo.c $(K1_SOURCES) $(wildcard k0/*.S) \
	$(K3_DRIVER_SOURCES) common/kmain.c $(COMMON_HW_SOURCES) $(SMP_SOURCES) $(KERNEL_STUB_SOURCES) $(DATA_SOURCES) \
	$(ALL_SERVER_SOURCES) $(COMMON_CUSTOM_SOURCES) \
	$(K1_TEST_SUITE) $(K2_TEST_SUITE) $(K3_TEST_SUITE) $(K4_TEST_SUITE)
# K1 test kernel
K1_TEST_C := $(filter-out k1/processes.c k1/syscall.c,$(wildcard k1/*.c)) \
	$(K1_TEST_SUITE) k1/test/test_main.c \
	k2/servers/nameserver/nameserver.c \
	$(K3_DRIVER_SOURCES) $(COMMON_HW_SOURCES) $(KERNEL_STUB_SOURCES) $(COMMON_CUSTOM_SOURCES)
K1_TEST_ASM := $(wildcard k0/*.S)
K1_TEST_OBJECTS := $(patsubst %.c,%.o,$(K1_TEST_C)) $(patsubst %.S,%.o,$(K1_TEST_ASM)) k1/syscall_test.o

# K2 test kernel
K2_TEST_C := $(filter-out k1/processes.c k1/syscall.c,$(wildcard k1/*.c)) \
	$(K2_TEST_SUITE) k2/test/test_main.c \
	$(K2_SERVER_SOURCES) \
	$(K3_DRIVER_SOURCES) $(COMMON_HW_SOURCES) $(KERNEL_STUB_SOURCES) $(COMMON_CUSTOM_SOURCES)
K2_TEST_ASM := $(wildcard k0/*.S)
K2_TEST_OBJECTS := $(patsubst %.c,%.o,$(K2_TEST_C)) $(patsubst %.S,%.o,$(K2_TEST_ASM)) k1/syscall_test.o

# K3 test kernel
K3_TEST_C := $(filter-out k1/processes.c k1/syscall.c,$(wildcard k1/*.c)) \
	$(K3_TEST_SUITE) k3/test/test_main.c \
	k2/servers/nameserver/nameserver.c $(K3_SERVER_SOURCES) \
	$(K3_DRIVER_SOURCES) $(COMMON_HW_SOURCES) $(KERNEL_STUB_SOURCES) $(COMMON_CUSTOM_SOURCES)
K3_TEST_ASM := $(wildcard k0/*.S)
K3_TEST_OBJECTS := $(patsubst %.c,%.o,$(K3_TEST_C)) $(patsubst %.S,%.o,$(K3_TEST_ASM)) k1/syscall_test.o

# K4 test kernel
K4_TEST_C := $(filter-out k1/processes.c k1/syscall.c,$(wildcard k1/*.c)) \
	$(K4_TEST_SUITE) k4/test/test_main.c \
	k2/servers/nameserver/nameserver.c $(K3_SERVER_SOURCES) \
	$(wildcard k4/servers/UART*/*.c) \
	k4/servers/io_notifier/io_notifier.c k4/servers/io_api/io_api.c \
	$(K3_DRIVER_SOURCES) $(COMMON_HW_SOURCES) $(KERNEL_STUB_SOURCES) $(COMMON_CUSTOM_SOURCES)
K4_TEST_ASM := $(wildcard k0/*.S)
K4_TEST_OBJECTS := $(patsubst %.c,%.o,$(K4_TEST_C)) $(patsubst %.S,%.o,$(K4_TEST_ASM)) k1/syscall_test.o

SMP_TEST_C := $(filter-out k1/processes.c k1/syscall.c,$(wildcard k1/*.c)) \
	k4/test/k4_smp_tests.c k4/test/smp_test_main.c \
	k2/servers/nameserver/nameserver.c k3/servers/clock/clockserver.c \
	k4/servers/interprocess_hub/interprocess_hub.c \
	k4/servers/interprocess_gateway/interprocess_gateway.c \
	$(K3_DRIVER_SOURCES) $(COMMON_HW_SOURCES) $(SMP_SOURCES) \
	$(KERNEL_STUB_SOURCES) $(COMMON_CUSTOM_SOURCES)
SMP_TEST_ASM := $(wildcard k0/*.S)
SMP_TEST_OBJECTS := $(patsubst %.c,%.o,$(SMP_TEST_C)) \
	$(patsubst %.S,%.o,$(SMP_TEST_ASM))


OBJECTS := $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(SOURCES)))
DEPENDS := $(patsubst %.c,%.d,$(patsubst %.S,%.d,$(SOURCES)))

K1_TEST_CFLAGS := $(CFLAGS) -DDEBUG=0 -DDEBUG_EXIT=0
$(filter-out k1/syscall_test.o,$(K1_TEST_OBJECTS)): CFLAGS := $(K1_TEST_CFLAGS)

K1_TEST_TIMEOUT := 15
K2_TEST_TIMEOUT := 20
K3_TEST_TIMEOUT := 60
K4_TEST_TIMEOUT := 20
SMP_TEST_TIMEOUT := 30

QEMU := $(shell \
	if [ -x /usr/local/bin/qemu-system-aarch64 ] && /usr/local/bin/qemu-system-aarch64 -M help 2>&1 | grep -q raspi4b; then \
		echo /usr/local/bin/qemu-system-aarch64; \
	elif [ -x /usr/bin/qemu-system-aarch64 ] && /usr/bin/qemu-system-aarch64 -M help 2>&1 | grep -q raspi4b; then \
		echo /usr/bin/qemu-system-aarch64; \
	elif [ -x $(HOME)/.local/qemu-rpi4/bin/qemu-system-aarch64 ] && $(HOME)/.local/qemu-rpi4/bin/qemu-system-aarch64 -M help 2>&1 | grep -q raspi4b; then \
		echo $(HOME)/.local/qemu-rpi4/bin/qemu-system-aarch64; \
	elif [ -x /usr/local/bin/qemu-system-aarch64 ]; then \
		echo /usr/local/bin/qemu-system-aarch64; \
	elif [ -x /usr/bin/qemu-system-aarch64 ]; then \
		echo /usr/bin/qemu-system-aarch64; \
	else \
		echo qemu-system-aarch64; \
	fi)
QEMU_MACHINE := raspi4b
QEMU_CPU := cortex-a72
QEMU_FLAGS := -M $(QEMU_MACHINE) -cpu $(QEMU_CPU) -m 2G -display none -serial mon:stdio

KERNEL_ELF := $(OUT)/0-d273liu.elf
KERNEL_IMG := $(OUT)/0-d273liu.img
K1_TEST_ELF := $(OUT)/k1-test.elf
K1_TEST_IMG := $(OUT)/k1-test.img
K2_TEST_ELF := $(OUT)/k2-test.elf
K2_TEST_IMG := $(OUT)/k2-test.img
K3_TEST_ELF := $(OUT)/k3-test.elf
K3_TEST_IMG := $(OUT)/k3-test.img
K4_TEST_ELF := $(OUT)/k4-test.elf
K4_TEST_IMG := $(OUT)/k4-test.img
SMP_TEST_ELF := $(OUT)/smp-test.elf
SMP_TEST_IMG := $(OUT)/smp-test.img

.PHONY: all clean run test test-k1 test-k2 test-k3 test-k4 test-smp qemu-rpi4 \
	clean-k1-test clean-k2-test clean-k3-test clean-k4-test clean-smp-test \
	
	0-d273liu.img 0-d273liu.elf k1-test.img k1-test.elf \
	k2-test.img k2-test.elf k3-test.img k3-test.elf k4-test.img k4-test.elf

all: 0-d273liu.img

# Repo-root artifact names (dev.sh / make invoke these from repo root)
0-d273liu.img: $(KERNEL_IMG)
0-d273liu.elf: $(KERNEL_ELF)
k1-test.img: $(K1_TEST_IMG)
k1-test.elf: $(K1_TEST_ELF)
k2-test.img: $(K2_TEST_IMG)
k2-test.elf: $(K2_TEST_ELF)
k3-test.img: $(K3_TEST_IMG)
k3-test.elf: $(K3_TEST_ELF)
k4-test.img: $(K4_TEST_IMG)
k4-test.elf: $(K4_TEST_ELF)
smp-test.img: $(SMP_TEST_IMG)
smp-test.elf: $(SMP_TEST_ELF)

clean:
	find $(REPO_ROOT) \( -name '*.o' -o -name '*.d' \) -not -path '*/.git/*' -delete
	rm -f $(KERNEL_ELF) $(KERNEL_IMG)
	rm -f $(K1_TEST_ELF) $(K1_TEST_IMG)
	rm -f $(K2_TEST_ELF) $(K2_TEST_IMG)
	rm -f $(K3_TEST_ELF) $(K3_TEST_IMG)
	rm -f $(K4_TEST_ELF) $(K4_TEST_IMG)
	rm -f $(SMP_TEST_ELF) $(SMP_TEST_IMG)

clean-k1-test:
	rm -f $(K1_TEST_OBJECTS) $(K1_TEST_ELF) $(K1_TEST_IMG)

clean-k2-test:
	rm -f $(K2_TEST_OBJECTS) $(K2_TEST_ELF) $(K2_TEST_IMG)

clean-k3-test:
	rm -f $(K3_TEST_OBJECTS) $(K3_TEST_ELF) $(K3_TEST_IMG)

clean-k4-test:
	rm -f $(K4_TEST_OBJECTS) $(K4_TEST_ELF) $(K4_TEST_IMG)

$(KERNEL_IMG): $(KERNEL_ELF)
	$(OBJCOPY) $< -O binary $@

$(KERNEL_ELF): $(OBJECTS) $(LINKER)
	$(CC) $(CFLAGS) $(filter-out %.ld,$^) -o $@ $(LDFLAGS)
	@$(OBJDUMP) -d $@ | fgrep -q q0 && printf "\n***** WARNING: SIMD INSTRUCTIONS DETECTED! *****\n\n" || true

$(K1_TEST_IMG): $(K1_TEST_ELF)
	$(OBJCOPY) $< -O binary $@

$(K1_TEST_ELF): $(K1_TEST_OBJECTS) $(LINKER)
	$(CC) $(CFLAGS) $(filter-out %.ld,$^) -o $@ $(LDFLAGS)
	@$(OBJDUMP) -d $@ | fgrep -q q0 && printf "\n***** WARNING: SIMD INSTRUCTIONS DETECTED! *****\n\n" || true

$(K2_TEST_IMG): $(K2_TEST_ELF)
	$(OBJCOPY) $< -O binary $@

$(K2_TEST_ELF): $(K2_TEST_OBJECTS) $(LINKER)
	$(CC) $(CFLAGS) $(filter-out %.ld,$(K2_TEST_OBJECTS)) -o $@ $(LDFLAGS)
	@$(OBJDUMP) -d $@ | fgrep -q q0 && printf "\n***** WARNING: SIMD INSTRUCTIONS DETECTED! *****\n\n" || true

$(K3_TEST_IMG): $(K3_TEST_ELF)
	$(OBJCOPY) $< -O binary $@

$(K3_TEST_ELF): $(K3_TEST_OBJECTS) $(LINKER)
	$(CC) $(CFLAGS) $(filter-out %.ld,$^) -o $@ $(LDFLAGS)
	@$(OBJDUMP) -d $@ | fgrep -q q0 && printf "\n***** WARNING: SIMD INSTRUCTIONS DETECTED! *****\n\n" || true

$(K4_TEST_IMG): $(K4_TEST_ELF)
	$(OBJCOPY) $< -O binary $@


clean-smp-test:
	rm -f $(SMP_TEST_OBJECTS) $(SMP_TEST_ELF) $(SMP_TEST_IMG)

$(SMP_TEST_IMG): $(SMP_TEST_ELF)
	$(OBJCOPY) $< -O binary $@

$(SMP_TEST_ELF): $(SMP_TEST_OBJECTS) $(LINKER)
	$(CC) $(CFLAGS) $(filter-out %.ld,$^) -o $@ $(LDFLAGS)
	@$(OBJDUMP) -d $@ | fgrep -q q0 && printf "\n***** WARNING: SIMD INSTRUCTIONS DETECTED! *****\n\n" || true

$(K4_TEST_ELF): $(K4_TEST_OBJECTS) $(LINKER)
	$(CC) $(CFLAGS) $(filter-out %.ld,$^) -o $@ $(LDFLAGS)
	@$(OBJDUMP) -d $@ | fgrep -q q0 && printf "\n***** WARNING: SIMD INSTRUCTIONS DETECTED! *****\n\n" || true

k1/syscall_test.o: k1/syscall.c Makefile
	$(CC) $(K1_TEST_CFLAGS) -MMD -MP -c $< -o $@

qemu-rpi4:
	@bash $(REPO_ROOT)/scripts/build-qemu-rpi4.sh

run: $(KERNEL_IMG)
	@if ! $(QEMU) -M help 2>&1 | grep -q $(QEMU_MACHINE); then \
		echo "QEMU lacks $(QEMU_MACHINE). Run: make qemu-rpi4 or ./dev.sh build"; \
		exit 1; \
	fi
	$(QEMU) $(QEMU_FLAGS) -kernel $(KERNEL_IMG)

test: test-k1

test-k1: $(K1_TEST_IMG)
	@if ! $(QEMU) -M help 2>&1 | grep -q $(QEMU_MACHINE); then \
		echo "QEMU lacks $(QEMU_MACHINE). Run: make qemu-rpi4"; \
		exit 1; \
	fi
	@echo "Running K1 tests under QEMU ($(QEMU_MACHINE))..."
	@timeout $(K1_TEST_TIMEOUT) $(QEMU) $(QEMU_FLAGS) -kernel $(K1_TEST_IMG) 2>&1 | tee /tmp/k1-qemu.log; \
	ec=$$?; \
	if grep -q K1_ALL_PASS /tmp/k1-qemu.log; then exit 0; \
	elif [ $$ec -eq 124 ]; then echo "K1 tests timed out after $(K1_TEST_TIMEOUT)s"; exit 1; \
	else echo "K1 tests did not report K1_ALL_PASS"; exit 1; fi

test-k2: $(K2_TEST_IMG)
	@if ! $(QEMU) -M help 2>&1 | grep -q $(QEMU_MACHINE); then \
		echo "QEMU lacks $(QEMU_MACHINE). Run: make qemu-rpi4"; \
		exit 1; \
	fi
	@echo "Running K2 tests under QEMU ($(QEMU_MACHINE))..."
	@timeout $(K2_TEST_TIMEOUT) $(QEMU) $(QEMU_FLAGS) -kernel $(K2_TEST_IMG) 2>&1 | tee /tmp/k2-qemu.log; \
	ec=$$?; \
	if grep -q K2_ALL_PASS /tmp/k2-qemu.log; then exit 0; \
	elif [ $$ec -eq 124 ]; then echo "K2 tests timed out after $(K2_TEST_TIMEOUT)s"; exit 1; \
	else echo "K2 tests did not report K2_ALL_PASS"; exit 1; fi

test-k3: $(K3_TEST_IMG)
	@if ! $(QEMU) -M help 2>&1 | grep -q $(QEMU_MACHINE); then \
		echo "QEMU lacks $(QEMU_MACHINE). Run: make qemu-rpi4"; \
		exit 1; \
	fi
	@echo "Running K3 tests under QEMU ($(QEMU_MACHINE))..."
	@timeout $(K3_TEST_TIMEOUT) $(QEMU) $(QEMU_FLAGS) -kernel $(K3_TEST_IMG) 2>&1 | tee /tmp/k3-qemu.log; \
	ec=$$?; \
	if grep -q K3_ALL_PASS /tmp/k3-qemu.log; then exit 0; \
	elif [ $$ec -eq 124 ]; then echo "K3 tests timed out after $(K3_TEST_TIMEOUT)s"; exit 1; \
	else echo "K3 tests did not report K3_ALL_PASS"; exit 1; fi

test-k4: $(K4_TEST_IMG)
	@if ! $(QEMU) -M help 2>&1 | grep -q $(QEMU_MACHINE); then \
		echo "QEMU lacks $(QEMU_MACHINE). Run: make qemu-rpi4"; \
		exit 1; \
	fi
	@echo "Running K4 tests under QEMU ($(QEMU_MACHINE))..."
	@timeout $(K4_TEST_TIMEOUT) $(QEMU) $(QEMU_FLAGS) -kernel $(K4_TEST_IMG) 2>&1 | tee /tmp/k4-qemu.log; \
	ec=$$?; \
	if grep -q K4_ALL_PASS /tmp/k4-qemu.log; then exit 0; \
	elif [ $$ec -eq 124 ]; then echo "K4 tests timed out after $(K4_TEST_TIMEOUT)s"; exit 1; \
	else echo "K4 tests did not report K4_ALL_PASS"; exit 1; fi


test-smp: $(SMP_TEST_IMG)
	@if ! $(QEMU) -M help 2>&1 | grep -q $(QEMU_MACHINE); then \
		echo "QEMU lacks $(QEMU_MACHINE). Run: make qemu-rpi4"; \
		exit 1; \
	fi
	@echo "Running SMP hub/mailbox tests under QEMU ($(QEMU_MACHINE))..."
	@timeout $(SMP_TEST_TIMEOUT) $(QEMU) $(QEMU_FLAGS) -kernel $(SMP_TEST_IMG) 2>&1 | tee /tmp/smp-qemu.log; \
	ec=$$?; \
	if grep -q SMP_ALL_PASS /tmp/smp-qemu.log; then exit 0; \
	elif [ $$ec -eq 124 ]; then echo "SMP tests timed out after $(SMP_TEST_TIMEOUT)s"; exit 1; \
	else echo "SMP tests did not report SMP_ALL_PASS"; exit 1; fi

data/asciiart.c: data/asciiart.txt Makefile
	@{ \
		echo '#include "asciiart.h"'; \
		echo 'const char asciiart[] ='; \
		awk '{ gsub(/\\/, "\\\\"); gsub(/"/, "\\\""); printf "\"%s\\n\"\n", $$0 }' $<; \
		echo '"";'; \
		echo 'const unsigned int asciiart_len = sizeof(asciiart) - 1;'; \
	} > $@

%.o: %.c Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

%.o: %.S Makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDS)
