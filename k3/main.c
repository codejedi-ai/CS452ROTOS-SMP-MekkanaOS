#include "rpi.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "gameserver.h"
#include "k2_time_tests.h"
#include "k2rps.h"
#include "gic.h"
void* STACK_EL0_START; // Maybe delete this later

int kmain(void *reg) {  

  STACK_EL0_START = reg; // Immediately calls this to store stack_end point as x0
  InitSys(reg);

  uart_init();
  uart_config_and_enable(CONSOLE, 115200, 0);
  uart_config_and_enable(MARKLIN, 2400, 1);
  
  char *logo = "            ___     ___     ___     ___   __   __   ___     ___   \r\n    o O O  |   \\   /   \\   | _ \\   / __|  \\ \\ / /  / _ \\   / __|  \r\n   o       | |) |  | - |   |   /  | (__    \\ V /  | (_) |  \\__ \\  \r\n  TS__[O]  |___/   |_|_|   |_|_\\   \\___|   _|_|_   \\___/   |___/  \r\n {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_| \"\"\" |_|\"\"\"\"\"|_|\"\"\"\"\"| \r\n./o--000\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\' \r\n";
  uart_printf(CONSOLE, "%s\r\n", logo);
  uart_printf(CONSOLE, "Testing inturrupt\r\nHello World I am d273liu\r\n");
  route_interrupt(99, 0);
  enable_interrupt(99);
  set_timerC3(50000000);
  // test adder
  // uart_printf(CONSOLE, "%u\r\n", &STACK_EL0_START);
  // uart_printf(CONSOLE, "%u\r\n", STACK_EL0_START);
  int tid = KernelCreate(10, nameserver, 0);
	uart_printf(CONSOLE,"nameserver Created: %u\r\n", tid);

  KernelCreate(1, first_task, 0); // Priority, Task, Parent // Parent of 0 means Kernel is parent
  // uart_printf(CONSOLE, "Process %u %u\r\n", p, p_1);
  Schedule();
  // U-Boot displays the return value from main - might be handy for debugging

  return 0;
}
