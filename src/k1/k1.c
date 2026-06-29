#include "k1.h"
#include "rpi.h"
#include "syscall.h"

/* ----------------------------------------------------------------------------
 * K1 first user task (matches the assignment spec).
 *
 * Creates four children:
 *   - two at LOWER priority (created first)
 *   - two at HIGHER priority (created second)
 * Prints "Created: <tid>" for each via busy-wait UART (uart_printf).
 * Exits after "FirstUserTask: exiting".
 *
 * Each child prints "tid=N parent=M", calls Yield(), prints the same line,
 * then exits. The ordering of output is the visible proof of priority-based
 * preemption.
 * -------------------------------------------------------------------------- */

static void k1_child(void)
{
    int my_tid     = MyTid();
    int parent_tid = MyParentTid();
    uart_printf(CONSOLE, "tid=%d parent=%d\r\n", my_tid, parent_tid);
    Yield();
    uart_printf(CONSOLE, "tid=%d parent=%d\r\n", my_tid, parent_tid);
    Exit();
}

void k1_first_user_task(void)
{
    int t;
    /* In CS452's priority scheme, *smaller* numeric value = higher priority,
       which is how this kernel orders the ready queue. Two children with a
       larger priority value (= lower precedence) created first. */
    t = Create(6, k1_child); uart_printf(CONSOLE, "Created: %d\r\n", t);
    t = Create(6, k1_child); uart_printf(CONSOLE, "Created: %d\r\n", t);
    /* Then two with a smaller priority value (= higher precedence). */
    t = Create(2, k1_child); uart_printf(CONSOLE, "Created: %d\r\n", t);
    t = Create(2, k1_child); uart_printf(CONSOLE, "Created: %d\r\n", t);
    uart_printf(CONSOLE, "FirstUserTask: exiting\r\n");
    Exit();
}

/* ----------------------------------------------------------------------------
 * K1 regression tests. Each returns 0 on PASS, non-zero on FAIL.
 * The boot self-test harness wires these into the overall pass/fail count.
 * -------------------------------------------------------------------------- */

static volatile int k1_yield_counter;

static void k1_yield_child(void)
{
    for (int i = 0; i < 5; i++) {
        k1_yield_counter++;
        Yield();
    }
    Exit();
}

static int k1_test_create_and_yield(void)
{
    k1_yield_counter = 0;
    int tid = Create(0, k1_yield_child);  /* higher priority than us */
    if (tid <= 0) {
        uart_printf(CONSOLE, "\033[31mK1: Create returned %d\033[37m\r\n", tid);
        return 1;
    }
    /* Yield enough to let the child finish all five iterations. */
    for (int i = 0; i < 20; i++) Yield();
    if (k1_yield_counter < 5) {
        uart_printf(CONSOLE, "\033[31mK1: yield_counter=%d expected 5\033[37m\r\n",
                    k1_yield_counter);
        return 1;
    }
    return 0;
}

static int k1_test_my_tid(void)
{
    int tid = MyTid();
    if (tid <= 0) {
        uart_printf(CONSOLE, "\033[31mK1: MyTid returned %d\033[37m\r\n", tid);
        return 1;
    }
    return 0;
}

int k1_self_tests(void)
{
    int fails = 0;
    fails += k1_test_create_and_yield();
    fails += k1_test_my_tid();
    return fails;
}
