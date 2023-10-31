#include "rpi.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "clockserver.h"
#include "ioserver.h"
#include "gameserver.h"
#include "k2TimeTests.h"
#include "traincont.h"
#include "k2rps.h"
#include "gic.h"
#include "processes.h"
void* STACK_EL0_START; // Maybe delete this later
#define CLOCKINTID 99
int kmain(void *reg) {  

  STACK_EL0_START = reg; // Immediately calls this to store stack_end point as x0
  InitSys(reg);

  uart_init();
  uart_config_and_enable(CONSOLE, 115200);
  uart_config_and_enable_marklin();
  
  
  
  set_timerC3(get_timerLO() + 10000);
  route_interrupt(CLOCKINTID, 0);
  enable_interrupt(CLOCKINTID);


  // enable and route the interupt
  route_interrupt(UARTINTER, 0);
  enable_interrupt(UARTINTER);
  enable_RX_and_TX();
  // INIT THE SERVERS AND NOTIFIERS
  int tid = KernelCreate(0, nameserver, 0);
  tid = KernelCreate(0, io_notifier, 0);
  tid = KernelCreate(0, clock_notifier, 0);
  uart_printf(CONSOLE, "io_notifier: tid = %d\r\n", tid);

	tid = KernelCreate(-1, idle, 0);
  uart_printf(CONSOLE, "idle: tid = %d\r\n", tid);
  
  uart_printf(CONSOLE, "FirstUserTask\r\n", tid);
  KernelCreate(10, FirstUserTask, 0); // Priority, Task, Parent // Parent of 0 means Kernel is parent
  // uart_printf(CONSOLE, "Process %u %u\r\n", p, p_1);
  
  
  
  
  Schedule();
  // U-Boot displays the return value from main - might be handy for debugging

  return 0;
}
