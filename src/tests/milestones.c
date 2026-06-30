#include "milestones.h"
#include "k1tests.h"
#include "k2tests.h"
#include "k3tests_suite.h"
#include "k4tests.h"
#include "shell.h"
#include "io_api.h"
#include "UART1_CONSOLE_server.h"
#include "display_server.h"
#include "marklin_worker.h"
#include "track_server.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"

static void wait_for_tid(const char *name)
{
	int tid = WhoIs(name);
	while (tid < 0) {
		Yield();
		tid = WhoIs(name);
	}
}

static void start_io_servers(void)
{
	int tid;

	tid = Create(0, UART1_CONSOLE_server);
	uart_printf(CONSOLE, UART1_CONSOLE_SERVER " tid: %d\r\n", tid);
	tid = Create(0, display_server);
	uart_printf(CONSOLE, "display_server tid: %d\r\n", tid);
	tid = Create(0, UART2_MARKLIN_server);
	uart_printf(CONSOLE, UART2_MARKLIN_SERVER " tid: %d\r\n", tid);
	tid = Create(0, io_notifier);
	uart_printf(CONSOLE, "io_notifier tid: %d\r\n", tid);

	wait_for_tid(UART1_CONSOLE_SERVER);
	wait_for_tid("display_server");
	wait_for_tid(UART2_MARKLIN_SERVER);
	wait_for_tid("io_notifier");
}

static void start_train_control(void)
{
	int tid;

	tid = Create(1, marklin_worker_read_notifier);
	uart_printf(CONSOLE, "marklin_worker_read_notifier tid: %d\r\n", tid);
	tid = Create(1, marklin_worker);
	uart_printf(CONSOLE, "marklin_worker tid: %d\r\n", tid);
	tid = Create(1, track_server);
	uart_printf(CONSOLE, "track_server tid: %d\r\n", tid);
	for (int i = 0; i < 32; i++)
		Yield();
}

static volatile int milestone_stage_done;

static void run_k2_milestone(void)
{
	boot_k2_tests();
	milestone_stage_done = 1;
	Exit();
}

static void run_k3_milestone(void)
{
	boot_k3_tests();
	milestone_stage_done = 1;
	Exit();
}

static void run_k4_milestone(void)
{
	boot_k4_tests();
	milestone_stage_done = 1;
	Exit();
}

static void run_milestone_stage(void (*task)(void))
{
	milestone_stage_done = 0;
	(void)Create(10, task);
	while (!milestone_stage_done)
		Yield();
}

void boot_milestones(void)
{
	uart_printf(CONSOLE, "\r\n=== DarcyOS milestone boot ===\r\n");

	uart_printf(CONSOLE, "--- milestone K1 ---\r\n");
	boot_k1_tests();
	uart_printf(CONSOLE, ">>> MILESTONE K1 REACHED <<<\r\n");

	uart_printf(CONSOLE, "--- milestone K2 ---\r\n");
	run_milestone_stage(run_k2_milestone);
	uart_printf(CONSOLE, ">>> MILESTONE K2 REACHED <<<\r\n");

	uart_printf(CONSOLE, "--- milestone K3 ---\r\n");
	run_milestone_stage(run_k3_milestone);
	uart_printf(CONSOLE, ">>> MILESTONE K3 REACHED <<<\r\n");
	for (int i = 0; i < 500; i++)
		Yield();

	uart_printf(CONSOLE, "--- milestone K4 ---\r\n");
	start_io_servers();
	run_milestone_stage(run_k4_milestone);
	uart_printf(CONSOLE, ">>> MILESTONE K4 REACHED <<<\r\n");

	uart_printf(CONSOLE, "\r\n=== Milestones complete — starting train control ===\r\n");
	start_train_control();

	uart_printf(CONSOLE, "=== Starting shell ===\r\n");
	uart_printf(CONSOLE, ">>> MILESTONE SHELL REACHED <<<\r\n");
	commands_shell();
	Exit();
}
