#include "rpi.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "clockserver.h"
#include "io_api.h"
#include "UART1_CONSOLE_server.h"
#include "display_server.h"
#include "io_notifier.h"
#include "interprocess_hub.h"
#include "interprocess_gateway.h"
#include "rotos_link.h"
#include "shell.h"
#include "gic.h"
#include "systimer.h"
#include "idle.h"
#include "config.h"
#include "smp.h"
#include "mailbox.h"
#include "mailbox_ipi.h"

void *STACK_EL0_START;

#define CLOCKINTID 99

void idle(void)
{
	while (1)
		asm("WFI");
	Exit();
}

static void kmain_common_servers(int with_uart)
{
	KernelCreate(0, nameserver, 0);
	KernelCreate(SCHED_LOWEST_PRIORITY, idle, 0);

	set_timerC3(get_timerLO() + 10000);
	route_interrupt_exclusive(CLOCKINTID, (uint8_t)smp_get_core_id());
	enable_interrupt(CLOCKINTID);
	KernelCreate(0, clock_notifier, 0);
	KernelCreate(0, clock_server, 0);

	KernelCreate(0, interprocess_hub, 0);

	if (with_uart) {
		route_interrupt_exclusive(UARTINTER, 0);
		enable_interrupt(UARTINTER);
		enable_RX_and_TX();

		KernelCreate(0, rotos_link_server, 0);
		KernelCreate(0, io_notifier, 0);
		KernelCreate(0, UART1_CONSOLE_server, 0);
		KernelCreate(0, display_server, 0);
		KernelCreate(TERMINAL_SHELL_PRIORITY, commands_shell, 0);
	}
}

int kmain(void *reg)
{
	if (reg != NULL)
		smp_init_cpu();

	if (smp_get_core_id() != 0) {
		smp_secondary_kmain(smp_get_core_id());
		return 0;
	}

	STACK_EL0_START = reg;
	InitSys(reg);

	uart_init();
	uart_config_and_enable(CONSOLE, 115200);

	gic_init();
	mailbox_init();
	mailbox_ipi_init();

	kmain_common_servers(1);

	uart_printf(CONSOLE, "\033[2J");
	uart_printf(CONSOLE,
		"[" ROTOS_OS_NAME "] SMP hub + ROTS network link (QEMU serial1)\r\n");

	smp_signal_boot_ready();
	smp_schedule_secondary_release();

	Schedule();
	return 0;
}
