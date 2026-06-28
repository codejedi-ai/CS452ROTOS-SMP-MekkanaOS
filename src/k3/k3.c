#include "k3.h"
#include "../rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "clockserver.h"

static int k3_test_time_delay(void)
{
    int cs = WhoIs("clock_server");
    if (cs <= 0) {
        uart_printf(CONSOLE, "\033[31mK3: clock_server not registered\033[37m\r\n");
        return 1;
    }
    int t0 = Time(cs);
    Delay(cs, 5);
    int t1 = Time(cs);
    if ((t1 - t0) < 5) {
        uart_printf(CONSOLE, "\033[31mK3: Delay(5) measured %d ticks\033[37m\r\n",
                    t1 - t0);
        return 1;
    }
    return 0;
}

int k3_self_tests(void)
{
    int fails = 0;
    fails += k3_test_time_delay();
    return fails;
}
