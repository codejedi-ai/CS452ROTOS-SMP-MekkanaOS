#include "boot_tests.h"
#include "rpi.h"
#include "syscall.h"
#include "k1.h"
#include "k2.h"
#include "k3.h"
#include "k4.h"

void commands_shell(void);  /* defined in k4/servers/shell/shell.c */

/*
 * Boot self-test runner.
 *
 * Calls into per-K test suites in order:
 *   K1: kernel primitives  (Create / Yield / Exit / MyTid / MyParentTid)
 *   K2: synchronous messaging + nameserver
 *   K3: clock server + interrupts
 *   K4: IO server + display
 *
 * Each k?_self_tests() returns its own failure count. We aggregate, print a
 * summary, and only launch the interactive shell if everything passed.
 */

static void announce(const char *k, int fails)
{
    if (fails == 0)
        uart_printf(CONSOLE,
                    "\033[32m[%s] PASS\033[37m\r\n", k);
    else
        uart_printf(CONSOLE,
                    "\033[31m[%s] FAIL (%d failure%s)\033[37m\r\n",
                    k, fails, fails == 1 ? "" : "s");
}

void boot_test_runner(void)
{
    uart_printf(CONSOLE, "\r\n\033[36m==== boot self-tests ====\033[37m\r\n");

    int total = 0;
    int f;

    uart_printf(CONSOLE, "\033[33m[K1] start: kernel primitives\033[37m\r\n");
    f = k1_self_tests();      total += f; announce("K1", f);

    uart_printf(CONSOLE, "\033[33m[K2] start: send/receive + nameserver\033[37m\r\n");
    f = k2_self_tests();      total += f; announce("K2", f);

    uart_printf(CONSOLE, "\033[33m[K3] start: clock + interrupts\033[37m\r\n");
    f = k3_self_tests();      total += f; announce("K3", f);

    uart_printf(CONSOLE, "\033[33m[K4] start: IO + display\033[37m\r\n");
    f = k4_self_tests();      total += f; announce("K4", f);

    uart_printf(CONSOLE,
        "\033[36m==== self-tests done: %d failure%s ====\033[37m\r\n",
        total, total == 1 ? "" : "s");

    uart_printf(CONSOLE, "\033[36m==== launching shell ====\033[37m\r\n");
    Create(4, commands_shell);
    Exit();
}
