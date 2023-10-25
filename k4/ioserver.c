#include "ioserver.h"
#include "rpi.h"
#include "syscall.h"
unsigned char uart_getc_interrupt(size_t line) {
  AwaitEvent(line);
  return uart_getc(line);
}