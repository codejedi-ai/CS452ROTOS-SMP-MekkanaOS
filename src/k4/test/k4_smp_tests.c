#include "k4_smp_tests.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "interprocess_hub.h"
#include "mailbox.h"
#include "config.h"
#include "smp.h"

static int smp_pass_count;
static int smp_fail_count;

static void smp_pass(const char *name)
{
	uart_printf(CONSOLE, "SMP_PASS: %s\r\n", name);
	smp_pass_count++;
}

static void smp_fail(const char *name)
{
	uart_printf(CONSOLE, "SMP_FAIL: %s\r\n", name);
	smp_fail_count++;
}

static void smp_check(int ok, const char *name)
{
	if (ok)
		smp_pass(name);
	else
		smp_fail(name);
}

static int wait_for_name(const char *name)
{
	for (int i = 0; i < 200; i++) {
		int tid = WhoIs(name);
		if (tid > 0)
			return tid;
		Yield();
	}
	return -1;
}

static void test_mailbox_count(void)
{
	smp_check(MAILBOX_COUNT == NUM_CORES, "mailbox_count_equals_num_cores");
}

static void test_hub_route_spoke_to_spoke(void)
{
	smp_check(mailbox_route_dest(1, 2) == SMP_HUB_CORE, "route_1_to_2_via_hub");
	smp_check(mailbox_route_dest(2, 3) == SMP_HUB_CORE, "route_2_to_3_via_hub");
	smp_check(mailbox_route_dest(3, 1) == SMP_HUB_CORE, "route_3_to_1_via_hub");
}

static void test_hub_route_core0_direct(void)
{
	smp_check(mailbox_route_dest(SMP_HUB_CORE, 2) == 2, "route_0_to_2_direct");
	smp_check(mailbox_route_dest(SMP_HUB_CORE, 3) == 3, "route_0_to_3_direct");
}

static void test_hub_route_spoke_to_hub(void)
{
	smp_check(mailbox_route_dest(1, SMP_HUB_CORE) == SMP_HUB_CORE,
	          "route_1_to_0");
	smp_check(mailbox_route_dest(3, SMP_HUB_CORE) == SMP_HUB_CORE,
	          "route_3_to_0");
}

static void test_hub_server_registered(void)
{
	int hub_tid = wait_for_name(INTERPROCESS_HUB_NAME);
	smp_check(hub_tid > 0, "interprocess_hub_registered");
}

void run_k4_smp_tests(void)
{
	smp_pass_count = 0;
	smp_fail_count = 0;

	test_mailbox_count();
	test_hub_route_spoke_to_spoke();
	test_hub_route_core0_direct();
	test_hub_route_spoke_to_hub();
	test_hub_server_registered();

	uart_printf(CONSOLE, "SMP: %d pass, %d fail\r\n", smp_pass_count, smp_fail_count);
	if (smp_fail_count == 0)
		uart_printf(CONSOLE, "SMP_ALL_PASS\r\n");
}

void k4_smp_tests_task(void)
{
	run_k4_smp_tests();
	Exit();
}
