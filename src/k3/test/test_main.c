#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "clockserver.h"
#include "gic.h"
#include "systimer.h"
#include "k3tests_suite.h"

void *STACK_EL0_START;

#define CLOCKINTID 99

int kmain(void *reg) {
	STACK_EL0_START = reg;
	InitSys(reg);

	uart_init();
	uart_config_and_enable(CONSOLE, 115200);

	set_timerC3(get_timerLO() + 10000);
	route_interrupt(CLOCKINTID, 0);
	enable_interrupt(CLOCKINTID);

	KernelCreate(0, nameserver, 0);
	KernelCreate(0, clock_notifier, 0);
	KernelCreate(0, clock_server, 0);
	KernelCreate(6, run_k3_tests, 0);
	Schedule();
	return 0;
}
