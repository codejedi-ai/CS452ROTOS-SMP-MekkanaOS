# Build entry point — all logic lives in src/Makefile.
.DEFAULT_GOAL := all

.PHONY: all clean hw qemu
all:
	@$(MAKE) -C src all

clean:
	@$(MAKE) -C src clean

hw qemu:
	@$(MAKE) -C src $@

%:
	@$(MAKE) -C src $@
