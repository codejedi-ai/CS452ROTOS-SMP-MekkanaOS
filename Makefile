# Build entry point — all logic lives in src/Makefile.
# Every build target runs `make clean` first (removes all .o/.d under the repo).
.DEFAULT_GOAL := all

.PHONY: all clean run
all:
	@$(MAKE) -C src clean
	@$(MAKE) -C src all

clean:
	@$(MAKE) -C src clean

run:
	@$(MAKE) -C src clean
	@$(MAKE) -C src all
	@bash qemu/run.sh

%:
	@if [ "$@" = "clean" ]; then \
		$(MAKE) -C src clean; \
	else \
		$(MAKE) -C src clean && $(MAKE) -C src $@; \
	fi
