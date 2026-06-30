#include "rpi.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "interprocess_gateway.h"
#include "idle.h"
#include "smp.h"
#include "gic.h"
#include "config.h"
#include "systimer.h"

extern char __proc_stacks_base[];

static void smp_idle(void)
{
	for (;;)
		asm("WFI");
	Exit();
}

static void smp_release_task(void)
{
	smp_boot_secondary_cores();
	Exit();
}

void smp_secondary_kmain(unsigned core_id)
{
	smp_set_core_id(core_id);
	smp_wait_boot_ready();

	gic_cpu_iface_enable();

	void *stack_base = (void *)(__proc_stacks_base +
	                            (uintptr_t)core_id * (uintptr_t)NUMPROCS * 0x10000u);
	InitSys(stack_base);

	/* Do not reroute shared GICD targets from a secondary — that would steal
	 * UART/clock from core 0. Timer PPI is per-core; distributor already enabled. */
	set_timerC3(get_timerLO() + 10000);

	KernelCreate(0, nameserver, 0);
	KernelCreate(0, interprocess_gateway, 0);
	KernelCreate(SCHED_LOWEST_PRIORITY, smp_idle, 0);

	Schedule();
	for (;;)
		asm("wfi");
}

void smp_schedule_secondary_release(void)
{
	/* Release after shell/UART (priority 20); idle is 255. */
	KernelCreate(25, smp_release_task, 0);
}
