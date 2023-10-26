#include "rpi.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "clockserver.h"
#include "gameserver.h"
#include "k2TimeTests.h"
#include "k2rps.h"
#include "gic.h"
void* STACK_EL0_START; // Maybe delete this later

int kmain(void *reg) {  

  STACK_EL0_START = reg; // Immediately calls this to store stack_end point as x0
  InitSys(reg);

  uart_init();
  uart_config_and_enable(CONSOLE, 115200);
  uart_config_and_enable_marklin();
  
  
  
  char *logo = "            ___     ___     ___     ___   __   __   ___     ___   \r\n    o O O  |   \\   /   \\   | _ \\   / __|  \\ \\ / /  / _ \\   / __|  \r\n   o       | |) |  | - |   |   /  | (__    \\ V /  | (_) |  \\__ \\  \r\n  TS__[O]  |___/   |_|_|   |_|_\\   \\___|   _|_|_   \\___/   |___/  \r\n {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_| \"\"\" |_|\"\"\"\"\"|_|\"\"\"\"\"| \r\n./o--000\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\' \r\n";
  uart_printf(CONSOLE, "%s\r\n", logo);
  uart_printf(CONSOLE, "Modified main to busywait \r\nHello World I am d273liu\r\n");
  route_interrupt(CLOCKINTID, 0);
  enable_interrupt(CLOCKINTID);


  // enable and route the interupt
  route_interrupt(UARTINTER, 0);
  enable_interrupt(UARTINTER);
  enable_RX_and_TX();
  // test adder
  // uart_printf(CONSOLE, "%u\r\n", &STACK_EL0_START);
  // uart_printf(CONSOLE, "%u\r\n", STACK_EL0_START);
  int tid = KernelCreate(0, nameserver, 0);
  // tid = KernelCreate(2000, clock_notifier, 0);
	// tid = KernelCreate(2000, clock_server, 0);

  KernelCreate(1, FirstUserTask, 0); // Priority, Task, Parent // Parent of 0 means Kernel is parent
  // uart_printf(CONSOLE, "Process %u %u\r\n", p, p_1);
  Schedule();
  // U-Boot displays the return value from main - might be handy for debugging

  return 0;
}
