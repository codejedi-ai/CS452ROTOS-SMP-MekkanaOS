#ifndef _rpi_h_
#define _rpi_h_ 1

#include <stdint.h>
#include <stddef.h>

// Serial line 1 on the RPi hat is used for the console
#define CONSOLE 1
#define TRAIN 2

void uart_putc(size_t line, unsigned char c);
unsigned char uart_getc(size_t line);
void uart_putl(size_t line, const char *buf, size_t blen);
void uart_puts(size_t line, const char *buf);
void uart_printf(size_t line, char *fmt, ...);
void uart_config_and_enable(size_t line, uint32_t baudrate, uint32_t stp2);
void uart_init();

int uart_rxc(size_t line);
int uart_cts(size_t line);

uint32_t stimer_getlo();
uint32_t stimer_gethi();
// Change these later to do interrupts as they are hardcoded
void stimer_snooze();
void stimer_wake();


#endif /* rpi.h */
