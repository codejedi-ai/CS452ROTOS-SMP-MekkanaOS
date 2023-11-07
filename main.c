#include "rpi.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "clockserver.h"
#include "ioserver.h"
#include "gameserver.h"
#include "marklin_worker.h"
#include "gic.h"
#include "trainnsol.h"
void* STACK_EL0_START; // Maybe delete this later
#define CLOCKINTID 99
#define CLOCKSERVERON 1
#define UARTSERVERON 1
int kmain(void *reg) {  

  STACK_EL0_START = reg; // Immediately calls this to store stack_end point as x0
  InitSys(reg);

  uart_init();
  uart_config_and_enable(CONSOLE, 115200);
  uart_config_and_enable_marklin();
  
  int tid = KernelCreate(0, nameserver, 0);
  tid = KernelCreate(-1, idle, 0); 
  #if CLOCKSERVERON == 1
  set_timerC3(get_timerLO() + 10000);
  route_interrupt(CLOCKINTID, 0);
  enable_interrupt(CLOCKINTID);
  tid = KernelCreate(0, clock_notifier, 0);
  tid = KernelCreate(0, clock_server, 0);
  #endif
  #if UARTSERVERON == 1
  // enable and route the interupt
  route_interrupt(UARTINTER, 0);
  enable_interrupt(UARTINTER);
  enable_RX_and_TX();


  // INIT THE IO-SERVERS AND NOTIFIERS
  tid = KernelCreate(0, io_notifier, 0);
  uart_printf(CONSOLE, "io_notifier tid: %d\r\n", tid);
  tid = KernelCreate(0, io_TXIC_MARKLIN_server, 0);
  uart_printf(CONSOLE, "io_TXIC_MARKLIN_server tid: %d\r\n", tid);
  tid = KernelCreate(0, io_RXIC_MARKLIN_server, 0);
  uart_printf(CONSOLE, "io_RXIC_MARKLIN_server tid: %d\r\n", tid);
  tid = KernelCreate(0, io_CTS_MARKLIN_server, 0);
  uart_printf(CONSOLE, "io_CTS_MARKLIN_server tid: %d\r\n", tid);
  #endif


  // tid = KernelCreate(0, switch_worker, 0);

  //uart_printf(CONSOLE, "FirstUserTask\r\n", tid);
  
  // KernelCreate(-3, FirstUserTask, 0);
  uart_printf(CONSOLE, "idle tid: %d\r\n", tid);
  // create first user task
  tid = KernelCreate(1, FirstUserTask, 0);
    // sensor servers
  tid = KernelCreate(1, switchSensorTrain_Server, 0);
  uart_printf(CONSOLE, "switchSensorTrain_Server tid: %d\r\n", tid);
  tid = KernelCreate(1, MCW_read_notifier, 0);
  uart_printf(CONSOLE, "MCW_read_notifier tid: %d\r\n", tid);
  tid = KernelCreate(1, MCW, 0);
  uart_printf(CONSOLE, "MCW tid: %d\r\n", tid);
  //uart_printf(CONSOLE, "MCW tid: %d\r\n", tid);

  // switch worker
  Schedule();
  // U-Boot displays the return value from main - might be handy for debugging

  return 0;
}
