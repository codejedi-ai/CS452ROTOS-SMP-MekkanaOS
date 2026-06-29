#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "clockserver.h"
#include "io_api.h"
#include "UART1_CONSOLE_server.h"
#include "gic.h"
#include "systimer.h"
#include "k4tests.h"
#include "idle.h"

void *STACK_EL0_START;
#define CLOCKINTID 99

void idle(void)
{
	while (1) {
		uint32_t runtime = GetRuntime();
		uint32_t kernelrt = GetKernelRuntime();
		uart_printf(CONSOLE, "\033[2;1H");
		uart_printf(CONSOLE, "idle: runprecentage = %u \% \r\n",
			    (100 * runtime) / kernelrt);
		asm("WFI");
	}
	Exit();
}

int kmain(void *reg)
{
	STACK_EL0_START = reg;
	InitSys(reg);

	uart_init();
	uart_config_and_enable(CONSOLE, 115200);
	uart_config_and_enable_marklin();

	KernelCreate(0, nameserver, 0);
	KernelCreate(IDLE_PRIORITY, idle, 0);

	set_timerC3(get_timerLO() + 10000);
	route_interrupt(CLOCKINTID, 0);
	enable_interrupt(CLOCKINTID);
	KernelCreate(0, clock_notifier, 0);
	KernelCreate(0, clock_server, 0);

	route_interrupt(UARTINTER, 0);
	enable_interrupt(UARTINTER);
	enable_RX_and_TX();

	KernelCreate(0, UART2_MARKLIN_server, 0);
	KernelCreate(0, io_notifier, 0);

	KernelCreate(2, run_k4_tests, 0);
	Schedule();
	while (1) {
		asm("wfi");
	}
	return 0;
}
