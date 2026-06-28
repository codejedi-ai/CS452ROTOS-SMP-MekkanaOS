#ifndef _AUXUART_H_
#define _AUXUART_H_ 1

#include <stdint.h>
#include <stddef.h>

/*
 * BCM2835 AUX mini-UART driver, polled.
 *
 * Used as the Marklin transport when running under QEMU (`MARKLIN_HW_UART3=0`)
 * because QEMU's raspi4b model only exposes PL011 UART0 and the AUX mini-UART
 * as -serial backends; it does NOT model UART3.
 *
 * On real Pi 4 hardware the Marklin link is PL011 UART3 (see rpi.c) and this
 * driver is unused.
 */

void aux_uart_init(void);
void aux_uart_putc(unsigned char c);
unsigned char aux_uart_getc(void);
int  aux_uart_rxc(void);    /* 1 if a byte is ready, else 0 */

#endif /* _AUXUART_H_ */
