#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "k1tests.h"

void *STACK_EL0_START;

int kmain(void *reg)
{
	STACK_EL0_START = reg;
	InitSys(reg);

	uart_init();
	uart_config_and_enable(CONSOLE, 115200);

	KernelCreate(0, nameserver, 0);
	KernelCreate(10, run_k1_tests, 0);
	Schedule();
	return 0;
}
