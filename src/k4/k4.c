#include "k4.h"
#include "../rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "../ui/display_server.h"

static int k4_test_display_server(void)
{
    int tid = WhoIs("display");
    if (tid <= 0) {
        uart_printf(CONSOLE, "\033[31mK4: display server not registered\033[37m\r\n");
        return 1;
    }
    display_puts("[K4] display reachable\r\n");
    return 0;
}

int k4_self_tests(void)
{
    int fails = 0;
    fails += k4_test_display_server();
    return fails;
}
