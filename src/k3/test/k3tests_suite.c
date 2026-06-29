#include "k3tests_suite.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "systimer.h"
#include "clockserver.h"

#define CLOCKINTID 99

static void k3_test_clock_proc(void) {
	char name[8] = "k3_test";
	int delay = 10;
	int numberOfDelays = 20;
	uint64_t tid = MyTid();

	Receive((int *)&tid, name, 8);
	Reply(tid, name, 8);
	Receive((int *)&tid, (char *)&delay, 4);
	Reply(tid, (char *)&delay, 4);
	Receive((int *)&tid, (char *)&numberOfDelays, 4);
	Reply(tid, (char *)&numberOfDelays, 4);
	RegisterAs(name);

	for (int i = 0; i < numberOfDelays; i++) {
		int cs = WhoIs("clock_server");
		if (cs < 0) {
			Exit();
		}
		Delay(cs, delay);
	}
	Exit();
}

static int32_t k3_init_clock_proc(uint8_t priority, char *clockname_buf, int delay, int numberOfDelays) {
	char clockname[8];
	for (int i = 0; i < 7; i++) {
		clockname[i] = clockname_buf[i];
	}
	clockname[7] = '\0';
	int tid = Create(priority, k3_test_clock_proc);
	if (tid < 0) {
		return -1;
	}
	if (Send(tid, clockname, 8, clockname, 8) < 0) {
		return -1;
	}
	if (Send(tid, (char *)&delay, 4, (char *)&delay, 4) < 0) {
		return -1;
	}
	if (Send(tid, (char *)&numberOfDelays, 4, (char *)&numberOfDelays, 4) < 0) {
		return -1;
	}
	return 0;
}

static int k3_pass_count;
static int k3_fail_count;

static void k3_pass(const char *name) {
	uart_printf(CONSOLE, "K3_PASS: %s\r\n", name);
	k3_pass_count++;
}

static void k3_fail(const char *name) {
	uart_printf(CONSOLE, "K3_FAIL: %s\r\n", name);
	k3_fail_count++;
}

static void k3_check(int ok, const char *name) {
	if (ok) {
		k3_pass(name);
	} else {
		k3_fail(name);
	}
}

static int wait_for_server(const char *name) {
	int tid = -1;
	for (int i = 0; i < 5000 && tid < 0; i++) {
		tid = WhoIs(name);
		Yield();
	}
	return tid;
}

static void test_clock_servers_registered(void) {
	int notifier = wait_for_server("clock_notifier");
	int server = wait_for_server("clock_server");
	k3_check(notifier >= 0, "clock_notifier_registered");
	k3_check(server >= 0, "clock_server_registered");
	k3_check(notifier != server, "clock_notifier_distinct_from_server");
}

static void test_time_returns_ticks(void) {
	int cs = wait_for_server("clock_server");
	if (cs < 0) {
		k3_fail("time_skipped_no_server");
		return;
	}
	int t0 = Time(cs);
	int t1 = Time(cs);
	k3_check(t0 >= 0, "time_non_negative");
	k3_check(t1 >= t0, "time_monotonic");
}

static void test_delay_advances_time(void) {
	int cs = wait_for_server("clock_server");
	if (cs < 0) {
		k3_fail("delay_skipped_no_server");
		return;
	}
	int t0 = Time(cs);
	int wake = Delay(cs, 3);
	int t1 = Time(cs);
	k3_check(wake > t0, "delay_wake_after_start");
	k3_check(t1 >= wake, "delay_time_after_wake");
}

static void test_delay_until(void) {
	int cs = wait_for_server("clock_server");
	if (cs < 0) {
		k3_fail("delayuntil_skipped_no_server");
		return;
	}
	int target = Time(cs) + 5;
	DelayUntil(cs, target);
	int after = Time(cs);
	k3_check(after >= target, "delayuntil_wake_at_target");
}

static void test_init_clock_proc(void) {
	k3_check(k3_init_clock_proc(5, "qt", 1, 1) == 0, "init_clock_proc_quick_client");
	Yield();
	Yield();
}

static void test_assignment_client_spawn(void) {
	static const struct {
		uint8_t priority;
		char name[8];
		int interval;
		int count;
		const char *label;
	} clients[] = {
		{3, "cl10", 10, 20, "assignment_client_p3"},
		{4, "cl23", 23, 9,  "assignment_client_p4"},
		{5, "cl33", 33, 6,  "assignment_client_p5"},
		{6, "cl71", 71, 3,  "assignment_client_p6"},
	};
	for (unsigned i = 0; i < sizeof(clients) / sizeof(clients[0]); i++) {
		char name[8];
		for (int j = 0; j < 8; j++) {
			name[j] = clients[i].name[j];
		}
		k3_check(
			k3_init_clock_proc(clients[i].priority, name, clients[i].interval, clients[i].count) == 0,
			clients[i].label);
	}
}

static void run_k3_tests_core(void) {
	k3_pass_count = 0;
	k3_fail_count = 0;

	uart_printf(CONSOLE, "\033[2J\033[H");
	uart_printf(CONSOLE, "=== K3 test suite ===\r\n");

	test_clock_servers_registered();
	test_time_returns_ticks();
	test_delay_advances_time();
	test_delay_until();
	test_init_clock_proc();
	test_assignment_client_spawn();

	uart_printf(CONSOLE, "=== K3 summary: %d passed, %d failed ===\r\n",
		k3_pass_count, k3_fail_count);
	if (k3_fail_count == 0) {
		uart_printf(CONSOLE, "K3_ALL_PASS\r\n");
	} else {
		uart_printf(CONSOLE, "K3_HAS_FAILURES\r\n");
	}
}

void run_k3_tests(void) {
	run_k3_tests_core();
	Exit();
}

void boot_k3_tests(void) {
	run_k3_tests_core();
}

int k3_self_tests(void)
{
	run_k3_tests();
	return 0;
}
