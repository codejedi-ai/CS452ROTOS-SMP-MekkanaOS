#include "rpi.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "clockserver.h"
#include "interprocess_hub.h"
#include "gic.h"
#include "systimer.h"
#include "idle.h"
#include "config.h"
#include "smp.h"
#include "mailbox.h"
#include "mailbox_ipi.h"
#include "k4_smp_tests.h"

void *STACK_EL0_START;

#define CLOCKINTID 99

static void test_idle(void)
{
	for (;;)
		asm("WFI");
	Exit();
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

	KernelCreate(0, nameserver, 0);
	KernelCreate(SCHED_LOWEST_PRIORITY, test_idle, 0);

	set_timerC3(get_timerLO() + 10000);
	route_interrupt_exclusive(CLOCKINTID, (uint8_t)smp_get_core_id());
	enable_interrupt(CLOCKINTID);
	KernelCreate(0, clock_notifier, 0);
	KernelCreate(0, clock_server, 0);

	KernelCreate(0, interprocess_hub, 0);
	KernelCreate(2, k4_smp_tests_task, 0);

	smp_signal_boot_ready();

	Schedule();
	return 0;
}
