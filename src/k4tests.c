#include "k4tests.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "clockserver.h"
#include "io_api.h"
#include "rotos_link.h"

static int k4_pass_count;
static int k4_fail_count;

static void k4_pass(const char *name)
{
	uart_printf(CONSOLE, "K4_PASS: %s\r\n", name);
	k4_pass_count++;
}

static void k4_fail(const char *name)
{
	uart_printf(CONSOLE, "K4_FAIL: %s\r\n", name);
	k4_fail_count++;
}

static void k4_check(int ok, const char *name)
{
	if (ok)
		k4_pass(name);
	else
		k4_fail(name);
}

static int console_tid;
static int io_notifier_tid;
static int rotos_link_server_tid;

static void wait_for_io_servers(void)
{
	for (int i = 0; i < 100; i++) {
		console_tid = WhoIs(UART1_CONSOLE_SERVER);
		io_notifier_tid = WhoIs("io_notifier");
		rotos_link_server_tid = WhoIs(ROTOS_LINK_SERVER_NAME);
		if (console_tid > 0 && io_notifier_tid > 0 && rotos_link_server_tid > 0)
			return;
		Yield();
	}
}

static void test_io_server_registration(void)
{
	wait_for_io_servers();
	k4_check(console_tid > 0, "console_server_registered");
	k4_check(io_notifier_tid > 0, "io_notifier_registered");
	k4_check(rotos_link_server_tid > 0, "rotos_link_registered");
}

static void test_io_server_unique_tids(void)
{
	k4_check(console_tid != io_notifier_tid, "console_notifier_distinct");
	k4_check(rotos_link_server_tid != console_tid, "link_console_distinct");
}

static void test_clock_delay_with_io(void)
{
	int clock_tid = WhoIs("clock_server");
	if (clock_tid <= 0) {
		k4_fail("clock_delay_io_server_missing");
		return;
	}
	int before = Time(clock_tid);
	int delay_ret = Delay(clock_tid, 2);
	int after = Time(clock_tid);
	k4_check(delay_ret >= 0, "clock_delay_io_returns");
	k4_check(after >= before + 2, "clock_delay_io_advances_time");
}

static void print_io_server_tids(void)
{
	wait_for_io_servers();
	uart_printf(CONSOLE, UART1_CONSOLE_SERVER " tid=%d\r\n", console_tid);
	uart_printf(CONSOLE, "io_notifier tid=%d\r\n", io_notifier_tid);
	uart_printf(CONSOLE, ROTOS_LINK_SERVER_NAME " tid=%d\r\n", rotos_link_server_tid);
}

static void run_k4_tests_core(int clear_before)
{
	if (clear_before)
		uart_printf(CONSOLE, "\033[2J\033[H");

	k4_pass_count = 0;
	k4_fail_count = 0;

	print_io_server_tids();
	test_io_server_registration();
	test_io_server_unique_tids();
	test_clock_delay_with_io();

	uart_printf(CONSOLE, "K4: %d pass, %d fail\r\n", k4_pass_count, k4_fail_count);
	if (k4_fail_count == 0)
		uart_printf(CONSOLE, "K4_ALL_PASS\r\n");
}

void run_k4_tests(void)
{
	run_k4_tests_core(1);
	Exit();
}

void boot_k4_tests(void)
{
	run_k4_tests_core(0);
}

int k4ExecuteCommands(char *command, char **num, int command_part_count)
{
	(void)command;

	if (strcmp_ret(num[0], "k4test", 0)) {
		(void)Create(2, run_k4_tests);
		uart_printf(CONSOLE, "K4 tests started\r\n");
		return 1;
	}
	if (strcmp_ret(num[0], "k4whois", 0)) {
		print_io_server_tids();
		return 1;
	}
	return -1;
}
