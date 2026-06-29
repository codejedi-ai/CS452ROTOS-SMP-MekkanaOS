#include "k4tests.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "clockserver.h"
#include "io_api.h"

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
	if (ok) {
		k4_pass(name);
	} else {
		k4_fail(name);
	}
}

static int uart2_marklin_tid;
static int io_notifier_tid;

static void wait_for_io_servers(void)
{
	for (int i = 0; i < 100; i++) {
		uart2_marklin_tid = WhoIs(UART2_MARKLIN_SERVER);
		io_notifier_tid = WhoIs("io_notifier");
		if (uart2_marklin_tid > 0 && io_notifier_tid > 0)
			return;
		Yield();
	}
}

static void test_io_server_registration(void)
{
	wait_for_io_servers();
	k4_check(uart2_marklin_tid > 0, "uart2_marklin_server_registered");
	k4_check(io_notifier_tid > 0, "io_notifier_registered");
}

static void test_io_server_unique_tids(void)
{
	k4_check(uart2_marklin_tid != io_notifier_tid, "marklin_notifier_distinct");
}

static void test_putc_single_byte(void)
{
	int ret = Putc(uart2_marklin_tid, MARKLIN, 'A');
	Yield();
	k4_check(ret == 'A', "putc_single_byte_return");
}

static void test_putc_burst(void)
{
	const char payload[] = "01234";
	int ok = 1;
	for (int i = 0; payload[i] != '\0'; i++) {
		int ret = Putc(uart2_marklin_tid, MARKLIN, (unsigned char)payload[i]);
		if (ret != payload[i]) {
			ok = 0;
			break;
		}
		Yield();
	}
	k4_check(ok, "putc_burst_five_bytes");
}

static void test_await_cts_immediate(void)
{
	uint8_t cur = (uint8_t)get_CTS(MARKLIN);
	int ret = awaitCTS(uart2_marklin_tid, MARKLIN, cur);
	Yield();
	k4_check(ret == cur, "await_cts_immediate");
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

static void test_whois_after_putc(void)
{
	int marklin_after = WhoIs(UART2_MARKLIN_SERVER);
	k4_check(marklin_after == uart2_marklin_tid, "whois_stable_after_putc");
}

static void test_getc_putc_same_server(void)
{
	k4_check(uart2_marklin_tid == WhoIs(UART2_MARKLIN_SERVER),
		 "getc_putc_share_marklin_server");
}

static void print_io_server_tids(void)
{
	wait_for_io_servers();
	uart_printf(CONSOLE, "io_notifier tid=%d\r\n", io_notifier_tid);
	uart_printf(CONSOLE, UART2_MARKLIN_SERVER " tid=%d\r\n", uart2_marklin_tid);
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
	test_getc_putc_same_server();
	test_putc_single_byte();
	test_putc_burst();
	test_await_cts_immediate();
	test_clock_delay_with_io();
	test_whois_after_putc();

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
	(void)command_part_count;

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

int k4_self_tests(void)
{
	run_k4_tests();
	return 0;
}
