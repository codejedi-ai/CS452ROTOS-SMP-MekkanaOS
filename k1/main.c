#include "rpi.h"
#include "syscall.h"
#include "processes.h"

// Serial line 1 on the RPi hat is used for the console
const size_t CONSOLE = 1;
const size_t TRAIN = 2;

void* STACK_EL0_START; // Maybe delete this later

int kmain(void *reg) {  

  STACK_EL0_START = reg; // Immediately calls this to store stack_end point as x0
  InitSys(reg);

  uart_init();
  uart_config_and_enable(CONSOLE, 115200, 0);
  uart_config_and_enable(TRAIN, 2400, 1);
  

  uart_printf(CONSOLE, "Hello World I am d273liu\r\n");
  
  // uart_printf(CONSOLE, "%u\r\n", &STACK_EL0_START);
  // uart_printf(CONSOLE, "%u\r\n", STACK_EL0_START);
  
  KernelCreate(2, first_task, 0); // Priority, Task, Parent // Parent of 0 means Kernel is parent
  // uart_printf(CONSOLE, "Process %u %u\r\n", p, p_1);
  Schedule();
  

  unsigned int counter=1;
  uart_printf(CONSOLE, "PI[%u]> ", counter++);
  char c = ' ';
  while (c != 'q') {
    c = uart_getc(CONSOLE);
    if (c == '\r') {
      uart_printf(CONSOLE, "\r\nPI[%u]> ", counter++);
    } else {
      uart_putc(CONSOLE, c);
    }
  }
  uart_puts(CONSOLE, "\r\n");

  // U-Boot displays the return value from main - might be handy for debugging

  return 0;
}
