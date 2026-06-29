#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "k2tests.h"

void *STACK_EL0_START;

int kmain(void *reg) {
	STACK_EL0_START = reg;
	InitSys(reg);

	uart_init();
	uart_config_and_enable(CONSOLE, 115200);

	KernelCreate(0, nameserver, 0);
	KernelCreate(6, run_k2_tests, 0);
	Schedule();
	return 0;
}
